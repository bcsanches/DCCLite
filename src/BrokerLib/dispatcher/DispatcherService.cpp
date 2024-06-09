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

#include <Log.h>

#include "FmtUtils.h"

#include "../sys/Broker.h"
#include "../dcc/DccLiteService.h"
#include "../sys/ScriptSystem.h"
#include "../dcc/Device.h"
#include "../dcc/VirtualSensorDecoder.h"

namespace dcclite::broker
{	
	class BaseSectionWrapper: public Object
	{
		public:
			BaseSectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor) :
				Object(name),
				m_clObject{ obj },
				m_rclSensor{sensor}
			{
				//empty
			}
			
			void Serialize(JsonOutputStream_t &stream) const override
			{
				Object::Serialize(stream);

				stream.AddStringValue("systemName", this->GetParent()->GetParent()->GetNameData());
				stream.AddIntValue("state", m_clObject["state"]);
			}

			void Reset()
			{
				m_clObject["reset"](m_clObject);
			}

			void OnStateUpdate()
			{
				const bool clear = m_clObject["is_clear"](m_clObject);

				m_rclSensor.SetSensorState(clear ? dcclite::DecoderStates::INACTIVE : dcclite::DecoderStates::ACTIVE);
			}

		protected:
			sol::table				m_clObject;
			VirtualSensorDecoder	&m_rclSensor;
	};

	class SectionWrapper: public BaseSectionWrapper
	{
		public:
			SectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor) :
				BaseSectionWrapper(name, obj, sensor)
			{
				//empty
			}

			inline const char *GetTypeName() const noexcept override
			{
				return "Dispatcher::Section";
			}					
	};

	class TSectionWrapper : public BaseSectionWrapper
	{
		public:
			TSectionWrapper(RName name, sol::table obj, VirtualSensorDecoder &sensor) :
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

	class DispatcherServiceImpl : public DispatcherService, public ScriptSystem::IScriptSupport, public IResettableService
	{
		public:
			DispatcherServiceImpl(RName name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~DispatcherServiceImpl() override;

			void Serialize(JsonOutputStream_t &stream) const override;		

			void IResettableService_ResetItem(RName name) override;

		private:			
			void RegisterSection(std::string_view name, sol::table obj);
			void RegisterTSection(std::string_view name, sol::table obj);

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

	DispatcherServiceImpl::DispatcherServiceImpl(RName name, Broker &broker, const rapidjson::Value& params, const Project& project):
		DispatcherService(name, broker, params, project),
		m_rclDccLite{ static_cast<DccLiteService &>(broker.ResolveRequirement(params["requires"].GetString())) },
		m_pSections{ static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{"sections"}))) }
	{				
		dcclite::Log::Info("[DispatcherServiceImpl] Init");		

		auto it = params.FindMember("device");
		if (it == params.MemberEnd())
		{
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::{}] device name not set, sensors will not be created", this->GetName()));
		}

		m_pclDevice = m_rclDccLite.TryFindDeviceByName(RName::Get(it->value.GetString()));
		if (!m_pclDevice)
		{
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::{}] device {} not found", this->GetName(), it->value.GetString()));
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
			"register_section",			&DispatcherServiceImpl::RegisterSection,
			"register_tsection",		&DispatcherServiceImpl::RegisterTSection,
			"on_section_state_change",	&DispatcherServiceImpl::OnSectionStateChange,
			"panic",					&DispatcherServiceImpl::Panic
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

	void DispatcherServiceImpl::RegisterSection(std::string_view name, sol::table obj)
	{
		auto &sensor = this->CreateSectionSensor(obj);

		auto wrapper = static_cast<SectionWrapper *>(m_pSections->AddChild(std::make_unique<SectionWrapper>( RName{name}, obj, sensor)));
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

	void DispatcherServiceImpl::IResettableService_ResetItem(RName name)
	{
		if(auto section = static_cast<BaseSectionWrapper *>(m_pSections->TryGetChild(name)))
			section->Reset();
		
		throw std::runtime_error(fmt::format("[DispatcherServiceImpl::IResettableService_ResetItem] Section {} not registered", name));		
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

		int address = obj["address"];
		std::string name = obj["name"];
		RName rname{ name };

		if (address > std::numeric_limits<uint16_t>::max())
			throw std::invalid_argument(fmt::format("[DispatcherServiceImpl::CreateSectionSensor] Invalid address {} for {}", address, name));

		auto &decoder = m_pclDevice->CreateInternalDecoder(VIRTUAL_SENSOR_DECODER_CLASSNAME, DccAddress{ static_cast<uint16_t>(address)}, rname, params);

		return static_cast<VirtualSensorDecoder &>(decoder);
	}

	//
	//
	// DispatcherService
	//
	//

	DispatcherService::DispatcherService(RName name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)
	{
		//empty
	}

	std::unique_ptr<Service> DispatcherService::Create(RName name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<DispatcherServiceImpl>(name, broker, params, project);
	}
}
