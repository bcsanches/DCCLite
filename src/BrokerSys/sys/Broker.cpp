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

#include <magic_enum/magic_enum.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/schema.h>

#include <spdlog/logger.h>

#include <dcclite_shared/Parser.h>

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>
#include <dcclite/Util.h>

#include "BonjourService.h"
#include "Project.h"
#include "ServiceFactory.h"
#include "Thinker.h"
#include "SpecialFolders.h"
#include "ZeroConfSystem.h"

//win32 header leak
#undef GetObject

namespace dcclite::broker::sys
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

	static void ThrowParserError(const rapidjson::Document &doc, const char *rawDoc)
	{
		auto offset = doc.GetErrorOffset();

		auto lineCount = dcclite::StrCountLines(rawDoc, offset);
		auto view = std::string_view(rawDoc + offset);

		auto msg = fmt::format("[Broker::Broker] Error {} parsing at line {}: {}", magic_enum::enum_name(doc.GetParseError()), lineCount, view);
		dcclite::Log::Error("{}", msg);

		throw std::runtime_error(msg.c_str());
	}

	Broker::Broker(dcclite::fs::path projectPath):		
		FolderObject{ RName{"root"} }
	{	
		BenchmarkLogger benchmark{ "Broker", "Contructor" };

		Project::SetWorkingDir(std::move(projectPath));

		constexpr auto initServices = R"JSON(
			[
				{
					"class":"CmdHostService",
					"name":"cmdHost"
				},
				{
					"class":"TerminalService",
					"name":"terminal",
					"requires":"$cmdHost"
				},
				{
					"class":"ScriptService",
					"name":"scriptService"
				}
			]
		)JSON";

		/*		
				{
					"class":"BonjourService",
					"name":"bonjour"					
				}
		
		*/
		
		{
			BenchmarkLogger benchmark{ "Broker", "Load init services" };

			rapidjson::Document data;
			data.Parse(initServices);

			if (data.HasParseError())
			{
				ThrowParserError(data, initServices);			
			}

			this->LoadServices(data.GetArray());
		}		

		{
			BenchmarkLogger loadTime{ "Broker", "LoadConfig" };

			this->LoadConfig();
		}

		//Start after load, so project name is already loaded
		ZeroConfSystem::Start(Project::GetName());
	}

	void Broker::SignalExecutiveChangeStart()
	{
		this->VisitServices([this](auto &item)
			{
				if (auto *s = dynamic_cast<IExecutiveClientService *>(&item))
					s->OnExecutiveChangeStart();

				return true;
			}
		);
	}

	void Broker::SignalExecutiveChangeEnd()
	{
		this->VisitServices([this](auto &item)
			{
				if (auto *s = dynamic_cast<IExecutiveClientService *>(&item))
					s->OnExecutiveChangeEnd();

				return true;
			}
		);
	}

	Broker::~Broker()
	{
		this->VisitServices([this](auto &item)
			{
				if (auto *s = dynamic_cast<IPostLoadService *>(&item))
					s->OnUnload();

				return true;
			}
		);

		ZeroConfSystem::Stop();

		//
		//cleanup everything while we still have our members live...
		//Devices may need to still acess m_clProject data during destruction, so make sure they go first
		//Do this on Services folder instead of destroying it, because some services (Terminal) will try to access Services folder on destruction
		this->RemoveAllChildren();
	}

	void Broker::LoadServices(const rapidjson::Value &servicesDataArray)
	{
		std::vector<std::pair<const rapidjson::Value *, const ServiceFactory *>> pendingServices;

		for (auto &serviceData : servicesDataArray.GetArray())
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

				this->AddService(std::move(service));
			}
		}

		for (auto &serviceData : pendingServices)
		{
			std::unique_ptr<Service> service{ CreateBrokerService(*serviceData.second, *this, *serviceData.first) };

			if (!service)
				continue;

			this->AddService(std::move(service));
		}
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

		dcclite::Log::Debug("[Broker] [LoadConfig] Processing config services array entries: {}", services.Size());				

		this->LoadServices(services);

		this->VisitServices([this](auto &item)
			{
				if (auto *s = dynamic_cast<IPostLoadService *>(&item))
					s->OnLoadFinished();

				return true;
			}
		);
	}

	Service &Broker::ResolveRequirement(std::string_view requirement)
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
			this->VisitChildren([&token, &result](auto &obj)
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

	Service *Broker::TryFindService(RName name)
	{
		return static_cast<Service *>(this->TryGetChild(name));
	}

	IObject *Broker::AddChild(std::unique_ptr<Object> obj)
	{		
		throw std::runtime_error(fmt::format("[Broker::AddChild] Only services are acceptable, not {} - {}", obj->GetTypeName(), obj->GetName()));		
	}

	void Broker::AddService(std::unique_ptr<Service> service)
	{	
		FolderObject::AddChild(std::move(service));
	}
}

