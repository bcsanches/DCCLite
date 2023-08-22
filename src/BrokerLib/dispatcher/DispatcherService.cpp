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

#include "../sys/Broker.h"
#include "../sys/ScriptService.h"
#include "../dcc/DccLiteService.h"

namespace dcclite::broker
{	
	class BaseSectionWrapper: public IObject
	{
		public:
			BaseSectionWrapper(std::string name, sol::table obj) :
				IObject(name),
				m_clObject{ obj }
			{
				//empty
			}
			
			void Serialize(JsonOutputStream_t &stream) const override
			{
				IObject::Serialize(stream);

				stream.AddStringValue("systemName", this->GetParent()->GetParent()->GetName());
				stream.AddIntValue("state", m_clObject["state"]);
			}

			void Reset()
			{
				m_clObject["reset"](m_clObject);
			}

		protected:
			sol::table m_clObject;
	};

	class SectionWrapper: public BaseSectionWrapper
	{
		public:
			SectionWrapper(std::string name, sol::table obj) :
				BaseSectionWrapper(name, obj)
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
			TSectionWrapper(std::string name, sol::table obj) :
				BaseSectionWrapper(name, obj)
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

	class DispatcherServiceImpl : public DispatcherService, public ScriptService::IScriptSupport, public IResettableService
	{
		public:
			DispatcherServiceImpl(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~DispatcherServiceImpl() override;

			void Serialize(JsonOutputStream_t &stream) const override;		

			void IResettableService_ResetItem(std::string_view name) override;

		private:			
			void RegisterSection(std::string_view name, sol::table obj);
			void RegisterTSection(std::string_view name, sol::table obj);

			void OnSectionStateChange(sol::table obj, int newState);

			void IScriptSupport_RegisterProxy(sol::table &table) override;

			void IScriptSupport_OnVMInit(sol::state &state) override;
			void IScriptSupport_OnVMFinalize(sol::state &state) override;

			void Panic(sol::table src, const char *reason);

		private:
			sigslot::scoped_connection m_slotScriptVMInit;
			sigslot::scoped_connection m_slotScriptVMFinalize;			

			DccLiteService &m_rclDccLite;

			FolderObject *m_pSections;
	};	

	DispatcherServiceImpl::DispatcherServiceImpl(const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
		DispatcherService(name, broker, params, project),
		m_rclDccLite{ static_cast<DccLiteService &>(broker.ResolveRequirement(params["requires"].GetString())) }
	{				
		dcclite::Log::Info("[DispatcherServiceImpl] Started");		

		m_pSections = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>("sections")));		
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
		auto enumerator = m_pSections->GetEnumerator();

		while (enumerator.MoveNext())
		{
			auto current = enumerator.GetCurrent();
			this->NotifyItemDestroyed(*current);
		}
		
		m_pSections->RemoveAllChildren();
	}

	void DispatcherServiceImpl::IScriptSupport_RegisterProxy(sol::table &table)
	{
		table[this->GetName()] = std::ref(*this);
	}

	void DispatcherServiceImpl::RegisterTSection(std::string_view name, sol::table obj)
	{
#if 1
		auto wrapper = static_cast<TSectionWrapper *>(m_pSections->AddChild(std::make_unique<TSectionWrapper>(std::string{ name }, obj)));
		obj["dispatcher_handler"] = static_cast<BaseSectionWrapper *>(wrapper);

		this->NotifyItemCreated(*wrapper);
#endif
	}


	void DispatcherServiceImpl::RegisterSection(std::string_view name, sol::table obj)
	{
		auto wrapper = static_cast<SectionWrapper *>(m_pSections->AddChild(std::make_unique<SectionWrapper>(std::string{name}, obj)));
		obj["dispatcher_handler"] = static_cast<BaseSectionWrapper *>(wrapper);

		this->NotifyItemCreated(*wrapper);
	}

	void DispatcherServiceImpl::OnSectionStateChange(sol::table obj, int newState)
	{
#if 1
		std::string name = obj["name"];
		auto section = static_cast<BaseSectionWrapper *>(m_pSections->TryGetChild(name));
		if (section == nullptr)
			throw std::runtime_error(fmt::format("[DispatcherServiceImpl::OnSectionStateChange] Section {} not registered", name));
#endif		

		this->NotifyItemChanged(*section);

		//section->OnStateChange(newState);
	}

	void DispatcherServiceImpl::IResettableService_ResetItem(std::string_view name)
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

	//
	//
	// DispatcherService
	//
	//

	DispatcherService::DispatcherService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)
	{
		//empty
	}

	std::unique_ptr<Service> DispatcherService::Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<DispatcherServiceImpl>(name, broker, params, project);
	}
}
