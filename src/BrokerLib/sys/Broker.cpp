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

#include <dcclite_shared/Parser.h>

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>


#include "BonjourService.h"
#include "Project.h"
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
		auto className = dcclite::json::GetString(data, "class", "service block");
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

	static std::unique_ptr<Service> CreateBrokerService(const ServiceFactory &factory, Broker &broker, const rapidjson::Value &data)
	{		
		RName name{ dcclite::json::GetString(data, "name", "service block") };

		dcclite::Log::Info("[Broker] [CreateBrokerService] Creating DccLite Service: {}", name);

		try
		{							
			return factory.Create(name, broker, data);			
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
		FolderObject{ RName{"root"} }
	{	
		BenchmarkLogger benchmark{ "Broker", "Contructor" };

		Project::SetWorkingDir(std::move(projectPath));

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

		{
			BenchmarkLogger loadTime{ "Broker", "LoadConfig" };

			this->LoadConfig();
		}

		{
			BenchmarkLogger script{ "Broker", "ScriptSystem" };

			ScriptSystem::Start(*this);
		}

		//Start after load, so project name is already loaded
		ZeroConfSystem::Start(Project::GetName());
	}

	void Broker::SignalExecutiveChangeStart()
	{
		ScriptSystem::Stop();
	}

	void Broker::SignalExecutiveChangeEnd()
	{
		ScriptSystem::Start(*this);
	}

	Broker::~Broker()
	{
		ScriptSystem::Stop();

		ZeroConfSystem::Stop();

		//
		//cleanup everything while we still have our members live...
		//Devices may need to still acess m_clProject data during destruction, so make sure they go first
		//Do this on Services folder instead of destroying it, because some services (Terminal) will try to access Services folder on destruction
		m_pServices->RemoveAllChildren();
	}	

	void Broker::LoadConfig()
	{		
		const auto configFileName = Project::GetFilePath("broker.config.json");
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
		
		Project::SetName(dcclite::json::GetString(data, "name", "broker"));

		const auto &services = dcclite::json::GetArray(data, "services", "broker");		

		dcclite::Log::Info("[Broker] [LoadConfig] Loaded config {}", configFileNameStr);

		if(dcclite::json::TryGetDefaultBool(data, "bonjourService", false))
			m_pServices->AddChild(BonjourService::Create(RName{ BONJOUR_SERVICE_NAME }, *this));

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
				auto service{ CreateBrokerService(factory, *this, serviceData) };

				if (!service)
					continue;

				m_pServices->AddChild(std::move(service));
			}
		}

		for (auto &serviceData : pendingServices)
		{
			std::unique_ptr<Service> service{ CreateBrokerService(*serviceData.second, *this, *serviceData.first)};
			
			if (!service)
				continue;

			m_pServices->AddChild(std::move(service));
		}		
	}

	Service &Broker::ResolveRequirement(std::string_view requirement) const
	{
		Parser parser{ StringView{requirement} };
		
		auto token = parser.GetToken();

		if (token.m_kToken == Tokens::VARIABLE_NAME)
		{
			auto name = RName::Get(token.m_svData);
			if (auto *service = this->TryFindService(name))
				return *service;

			throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Requested service {} not found", requirement));
		}
		else if (token.m_kToken == Tokens::ID)
		{
			Service *result = nullptr;
			m_pServices->VisitChildren([&token, &result](auto &obj)
				{					
					if (token.m_svData.Compare(obj.GetTypeName()) == 0)
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

