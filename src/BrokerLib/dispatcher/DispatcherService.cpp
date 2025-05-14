// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "DispatcherService.h"

#include <stdexcept>
#include <memory>

#include <dcclite/Log.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>

#include "../dcc/DccLiteService.h"
#include "../dcc/Device.h"
#include "../dcc/IResettableObject.h"
#include "../dcc/VirtualSensorDecoder.h"

#include "../sys/Broker.h"
#include "../sys/ServiceFactory.h"
#include "../sys/ScriptSystem.h"

namespace dcclite::broker
{
	class BaseSectionWrapper: public Object, public IResettableObject
	{
		public:
			BaseSectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor):
				Object(name),
				m_clObject{ obj },
				m_rclSensor{ sensor }
			{
				//empty
			}

			void Serialize(JsonOutputStream_t &stream) const override
			{
				Object::Serialize(stream);

				stream.AddStringValue("ownerPath", this->GetParent()->GetParent()->GetPath().string());
				stream.AddIntValue("state", m_clObject["state"]);
			}

			void Reset() override
			{
				m_clObject["reset"](m_clObject);
			}

			void OnStateUpdate()
			{
				const bool clear = m_clObject["is_clear"](m_clObject);

				m_rclSensor.SetSensorState(clear ? dcclite::DecoderStates::INACTIVE : dcclite::DecoderStates::ACTIVE);
			}

			inline sol::table GetScriptObject() noexcept
			{
				return m_clObject;
			}

		protected:
			sol::table				m_clObject;
			VirtualSensorDecoder	&m_rclSensor;
	};

	class SectionWrapper: public BaseSectionWrapper
	{
		public:
			SectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor):
				BaseSectionWrapper(name, obj, sensor)
			{
				//empty
			}

			inline const char *GetTypeName() const noexcept override
			{
				return "Dispatcher::Section";
			}
	};

	class TSectionWrapper: public BaseSectionWrapper
	{
		public:
			TSectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor):
				BaseSectionWrapper(name, obj, sensor)
			{
				//empty
			}

			inline const char *GetTypeName() const noexcept override
			{
				return "Dispatcher::TSection";
			}

			void Serialize(JsonOutputStream_t &stream) const override
			{
				BaseSectionWrapper::Serialize(stream);

				//stream.AddStringValue("turnout", m_clObject["turnout"]);
			}
	};

	///////////////////////////////////////////////////////////////////////////////
	//
	// DispatcherServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	class DispatcherServiceImpl: public DispatcherService, public ScriptSystem::IScriptSupport
	{
		public:
			typedef DccLiteService Requirement_t;

			DispatcherServiceImpl(RName name, Broker &broker, const rapidjson::Value &params, DccLiteService &dep);
			~DispatcherServiceImpl() override;

			void Serialize(JsonOutputStream_t &stream) const override;			

		private:
			void RegisterSection(std::string_view name, sol::table obj);
			void RegisterTSection(std::string_view name, sol::table obj);

			sol::table TryGetSection(std::string_view name);

			void OnSectionStateChange(sol::table obj, int newState);

			void IScriptSupport_RegisterProxy(sol::table &table) override;

			void IScriptSupport_OnVMInit(sol::state &state) override;
			void IScriptSupport_OnVMFinalize(sol::state &state) override;

			void Panic(sol::table src, const char *reason);

			VirtualSensorDecoder &CreateSectionSensor(sol::table obj);

		private:
			sigslot::scoped_connection m_slotScriptVMInit;
			sigslot::scoped_connection m_slotScriptVMFinalize;

			DccLiteService &m_rclDccLite;

			FolderObject *m_pSections;
			Device *m_pclDevice = nullptr;
	};

	DispatcherServiceImpl::DispatcherServiceImpl(RName name, Broker &broker, const rapidjson::Value &params, DccLiteService &dep):
		DispatcherService(name, broker, params),
		m_rclDccLite{ dep },
		m_pSections{ static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{"sections"}))) }
	{
		dcclite::Log::Info("[DispatcherServiceImpl] Init");		

		auto deviceName = RName::TryGetName(json::GetString(params, "device", "DispatcherServiceImpl"));
		if (!deviceName)
		{
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::{}] device {} not registered", this->GetName(), deviceName));
		}

		m_pclDevice = m_rclDccLite.TryFindDeviceByName(deviceName);
		if (!m_pclDevice)
		{
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::{}] device {} not found", this->GetName(), name));
		}
	}


	DispatcherServiceImpl::~DispatcherServiceImpl()
	{
		//empty
	}

	void DispatcherServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		DispatcherService::Serialize(stream);
	}

	void DispatcherServiceImpl::IScriptSupport_OnVMInit(sol::state &sol)
	{
		sol.new_usertype<DispatcherServiceImpl>(
			"dispatcher_service",
			sol::no_constructor,
			"register_section", &DispatcherServiceImpl::RegisterSection,
			"register_tsection", &DispatcherServiceImpl::RegisterTSection,
			"on_section_state_change", &DispatcherServiceImpl::OnSectionStateChange,
			"get_section", &DispatcherServiceImpl::TryGetSection,
			"panic", &DispatcherServiceImpl::Panic
		);
	}

	void DispatcherServiceImpl::IScriptSupport_OnVMFinalize(sol::state &sol)
	{
		m_pSections->VisitChildren([this](auto &current)
			{
				this->NotifyItemDestroyed(current);

				return true;
			}
		);

		m_pSections->RemoveAllChildren();
	}

	void DispatcherServiceImpl::IScriptSupport_RegisterProxy(sol::table &table)
	{
		table[this->GetName().GetData()] = std::ref(*this);
	}

	void DispatcherServiceImpl::RegisterTSection(std::string_view name, sol::table obj)
	{
		auto &sensor = this->CreateSectionSensor(obj);

#if 1
		auto wrapper = static_cast<TSectionWrapper *>(m_pSections->AddChild(std::make_unique<TSectionWrapper>(RName{ name }, obj, sensor)));
		obj["dispatcher_handler"] = static_cast<BaseSectionWrapper *>(wrapper);

		this->NotifyItemCreated(*wrapper);
#endif
	}

	sol::table DispatcherServiceImpl::TryGetSection(std::string_view name)
	{
		auto section = static_cast<BaseSectionWrapper *>(m_pSections->TryGetChild(RName{name}));
		if (!section)
			return sol::nil;

		return section->GetScriptObject();
	}

	void DispatcherServiceImpl::RegisterSection(std::string_view name, sol::table obj)
	{
		auto &sensor = this->CreateSectionSensor(obj);

		auto wrapper = static_cast<SectionWrapper *>(m_pSections->AddChild(std::make_unique<SectionWrapper>(RName{ name }, obj, sensor)));
		obj["dispatcher_handler"] = static_cast<BaseSectionWrapper *>(wrapper);

		this->NotifyItemCreated(*wrapper);
	}

	void DispatcherServiceImpl::OnSectionStateChange(sol::table obj, int newState)
	{
#if 1
		std::string name = obj["name"];
		RName rname{ name };
		auto section = static_cast<BaseSectionWrapper *>(m_pSections->TryGetChild(rname));
		if (section == nullptr)
			throw std::runtime_error(fmt::format("[DispatcherServiceImpl::OnSectionStateChange] Section {} not registered", rname));
#endif		

		section->OnStateUpdate();
		this->NotifyItemChanged(*section);
	}

	void DispatcherServiceImpl::Panic(sol::table src, const char *reason)
	{
		BaseSectionWrapper *section = src["dispatcher_handler"];

		dcclite::Log::Error("[DispatcherService::Panic] Fatal error on section [{}]: {}", section->GetName(), reason);
	}

	VirtualSensorDecoder &DispatcherServiceImpl::CreateSectionSensor(sol::table obj)
	{
		rapidjson::Document json;

		auto &params = json.SetObject();
		
		//must assign to a string to make sure SOL reads a string
		std::string name = obj["name"];
		RName rname{ name };

		//was decoder created before?
		if (auto decoder = this->m_rclDccLite.TryFindDecoder(rname))
		{
			//Ok, use it... but make sure it is the correct type
			auto vdecoder = dynamic_cast<VirtualSensorDecoder *>(decoder);
			if(!vdecoder)
				throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::CreateSectionSensor] Decoder {} is not an virtual sensor, cannot continue. Check names!!!", name));

			return *vdecoder;
		}

		int address = obj["address"];
		if (address > std::numeric_limits<uint16_t>::max())
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::CreateSectionSensor] Invalid address {} for {}", address, name));

		//
		//no decoder, so create a new one
		auto &decoder = m_pclDevice->CreateInternalDecoder(VIRTUAL_SENSOR_DECODER_CLASSNAME, DccAddress{ static_cast<uint16_t>(address) }, rname, params);

		return static_cast<VirtualSensorDecoder &>(decoder);
	}

	//
	//
	// DispatcherService
	//
	//

	const char *DispatcherService::TYPE_NAME = "DispatcherService";

	void DispatcherService::RegisterFactory()
	{
		//empty
	}

	DispatcherService::DispatcherService(RName name, Broker &broker, const rapidjson::Value &params):
		Service(name, broker, params)
	{
		//empty
	}

	static GenericServiceWithDependenciesFactory<DispatcherServiceImpl> g_ServiceFactory;
}

