// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Broker.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/schema.h>

#include <spdlog/logger.h>

#include <Log.h>
#include <Parser.h>

#include <FmtUtils.h>

#include "BonjourService.h"
#include "JsonUtils.h"
#include "ServiceFactory.h"
#include "Thinker.h"
#include "ScriptSystem.h"
#include "SpecialFolders.h"
#include "ZeroConfSystem.h"

#include "../terminal/TerminalCmd.h"

//win32 header leak
#undef GetObject

namespace dcclite::broker
{
	static inline bool CheckIfServiceIsIgnorable(const rapidjson::Value &data) noexcept
	{
		return dcclite::json::TryGetDefaultBool(data, "ignoreOnLoadFailure", false);
	}

	static RName GetServiceClassName(const rapidjson::Value &data)
	{
		const char *className = dcclite::json::GetString(data, "class", "service block");
		if (auto rclassName = RName::TryGetName(className))
			return rclassName;
		
		throw std::runtime_error(fmt::format("[Broker] [CreateBrokerService] error: service type {} not registered", className));		
	}

	static ServiceFactory &FindServiceFactory(const rapidjson::Value &data)
	{
		auto className = GetServiceClassName(data);		
		if (auto factory = ServiceFactory::TryFindFactory(className))		
			return *factory;					

		throw std::runtime_error(fmt::format("[Broker] [CreateBrokerService] error: unknown service type {}", className));
	}

	static std::unique_ptr<Service> CreateBrokerService(const ServiceFactory &factory, Broker &broker, const rapidjson::Value &data, const Project &project)
	{		
		RName name{ dcclite::json::GetString(data, "name", "service block") };

		dcclite::Log::Info("[Broker] [CreateBrokerService] Creating DccLite Service: {}", name);

		try
		{							
			return factory.Create(name, broker, data, project);			
		}
		catch (std::exception &ex)
		{
			if (CheckIfServiceIsIgnorable(data))
			{				
				Log::Error("[Broker] [LoadConfig] Failed to load {}, but ignoring due to \"ignoreOnLoadFailure\" being set, exception: {}", name, ex.what());

				return nullptr;
			}

			throw;
		}
	}

	Broker::Broker(dcclite::fs::path projectPath):		
		FolderObject{ RName{"root"} },
		m_clProject(std::move(projectPath))
	{		
		{
			auto cmdHost = std::make_unique<TerminalCmdHost>();
			m_pclTerminalCmdHost = cmdHost.get();

			this->AddChild(std::move(cmdHost));
		}

		ServiceFactory::RegisterAll();

		using namespace dcclite;

		m_pServices = static_cast<FolderObject *>(this->AddChild(
			std::make_unique<FolderObject>(SpecialFolders::GetName(SpecialFolders::Folders::ServicesId)))
		);				

		this->LoadConfig();

		ScriptSystem::Start(*this, m_clProject);

		//Start after load, so project name is already loaded
		ZeroConfSystem::Start(m_clProject.GetName());
	}

	Broker::~Broker()
	{
		ScriptSystem::Stop();

		ZeroConfSystem::Stop();
	}	

	void Broker::LoadConfig()
	{		
		const auto configFileName = m_clProject.GetFilePath("broker.config.json");
		const auto configFileNameStr = configFileName.string();

		dcclite::Log::Info("[Broker] [LoadConfig] Trying to open {}", configFileNameStr);

		std::ifstream configFile(configFileName);

		if (!configFile)
		{
			throw std::runtime_error(fmt::format("[Broker] [LoadConfig] error: cannot open config file {}", configFileNameStr));
		}

		dcclite::Log::Info("[Broker] [LoadConfig] Opened {}, starting parser", configFileNameStr);

		rapidjson::IStreamWrapper isw(configFile);
		rapidjson::Document data;
		if (data.ParseStream(isw).HasParseError())
		{
			throw std::runtime_error(fmt::format("[Broker] [LoadConfig] {} is not a valid json", configFileNameStr));
		}		
		
		m_clProject.SetName(dcclite::json::GetString(data, "name", "broker"));

		const auto &services = dcclite::json::GetArray(data, "services", "broker");		

		dcclite::Log::Info("[Broker] [LoadConfig] Loaded config {}", configFileNameStr);

		if(dcclite::json::TryGetDefaultBool(data, "bonjourService", false))
			m_pServices->AddChild(BonjourService::Create(RName{ BONJOUR_SERVICE_NAME }, *this, m_clProject));

		dcclite::Log::Debug("[Broker] [LoadConfig] Processing config services array entries: {}", services.Size());				

		std::vector<std::pair<const rapidjson::Value *, const ServiceFactory *>> pendingServices;		

		for (auto &serviceData : services)
		{
			auto &factory = FindServiceFactory(serviceData);

			if (factory.HasDependencies())
			{
				pendingServices.push_back(std::make_pair(&serviceData, &factory));
			}
			else
			{
				auto service{ CreateBrokerService(factory, *this, serviceData, m_clProject) };

				if (!service)
					continue;

				m_pServices->AddChild(std::move(service));
			}
		}				

		for (auto &serviceData : pendingServices)
		{
			std::unique_ptr<Service> service{ CreateBrokerService(*serviceData.second, *this, *serviceData.first, m_clProject)};
			
			if (!service)
				continue;

			m_pServices->AddChild(std::move(service));
		}		
	}

	Service &Broker::ResolveRequirement(const char *requirement) const
	{
		Parser parser{ requirement };

		char reqName[128];
		auto token = parser.GetToken(reqName, sizeof(reqName));

		if (token == Tokens::VARIABLE_NAME)
		{
			auto name = RName::Get(reqName);
			if (auto *service = this->TryFindService(name))
				return *service;

			throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Requested service {} not found", requirement));
		}
		else if (token == Tokens::ID)
		{
			Service *result = nullptr;
			m_pServices->VisitChildren([reqName, &result](auto &obj)
				{					
					if (strcmp(reqName, obj.GetTypeName()) == 0)
					{
						result = static_cast<Service *>(&obj);

						return false;
					}

					return true;
				}
			);

			if (result)
				return *result;			

			throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Requested service of type {} not found", requirement));
		}
		
		throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Syntax error parsing requirement {} ", requirement));
	}

	Service *Broker::TryFindService(RName name) const
	{
		return static_cast<Service *>(m_pServices->TryGetChild(name));
	}
}

