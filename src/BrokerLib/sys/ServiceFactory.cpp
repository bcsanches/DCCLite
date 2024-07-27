// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ServiceFactory.h"

#include <stdexcept>

#include <JsonUtils.h>

#include "Broker.h"

#include "../dcc/DccLiteService.h"
#include "../dcc/DccppService.h"

#include "../dispatcher/DispatcherService.h"

#include "../ln/LoconetService.h"
#include "../ln/ThrottleService.h"

#include "../terminal/TerminalService.h"

namespace dcclite::broker
{
	//avoid static initialization hell...
	static ServiceFactory **GetHead()
	{
		static ServiceFactory *g_pclHead = nullptr;

		return &g_pclHead;
	}

	ServiceFactory *ServiceFactory::TryFindFactory(RName name) noexcept
	{
		for (auto *p = *GetHead(); p; p = p->m_pclNext)
		{
			if (p->m_clClassName == name)
				return p;
		}

		return nullptr;
	}

	ServiceFactory::ServiceFactory(RName className):
		m_clClassName{ className }		
	{
		if (!m_clClassName)
			throw std::invalid_argument("[ServiceFactory::ServiceFactory] className required");		
	}

	void ServiceFactory::Register()
	{
		auto head = GetHead();

		m_pclNext = *head;
		*head = this;
	}

	void ServiceFactory::RegisterAll()
	{
		//just touch all to register the factories
		//static lib does initialize static variables without this... hack??
		DccLiteService::RegisterFactory();
		DccppService::RegisterFactory();
		DispatcherService::RegisterFactory();
		LoconetService::RegisterFactory();		
		TerminalService::RegisterFactory();
		ThrottleService::RegisterFactory();
	}

	//
	//
	//
	//
	//
	
	DefaultServiceFactory::DefaultServiceFactory(RName className, FactoryProc_t proc):
		ServiceFactory{ className },
		m_pfnProc{ proc }		
	{		
		if (!proc)
			throw std::invalid_argument("[ServiceFactory::ServiceFactory] proc required");

		this->Register();
	}

	//
	//
	//
	//
	//

	ServiceWithDependenciesFactory::ServiceWithDependenciesFactory(RName className, const char *defaultRequirement, FactoryProc_t proc):
		ServiceFactory{ className },
		m_pfnProc{ proc },
		m_pszDefaultRequirement{ defaultRequirement }
	{
		if (!proc)
			throw std::invalid_argument("[ServiceFactory::ServiceFactory] proc required");

		if(!defaultRequirement)
			throw std::invalid_argument("[ServiceFactory::ServiceFactory] defaultRequirement required");

		this->Register();
	}

	std::unique_ptr<Service> ServiceWithDependenciesFactory::Create(RName name, Broker &broker, const rapidjson::Value &data, const Project &project) const
	{
		auto requirementId = dcclite::json::TryGetDefaultString(
			data,
			"requires",
			m_pszDefaultRequirement
		);

		auto &requirement = broker.ResolveRequirement(requirementId);

		return m_pfnProc(name, broker, data, project, requirement);
	}
}


