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
#include <spdlog/logger.h>

#include <Log.h>
#include <Parser.h>

#include <FmtUtils.h>

#include "../dcc/DccLiteService.h"
#include "../dcc/DccppService.h"

#include "../dispatcher/DispatcherService.h"

#include "../ln/LoconetService.h"
#include "../ln/ThrottleService.h"

#include "../terminal/TerminalCmd.h"
#include "../terminal/TerminalService.h"

#include "BonjourService.h"

#include "Thinker.h"
#include "ScriptSystem.h"
#include "SpecialFolders.h"
#include "ZeroConfSystem.h"

//win32 header leak
#undef GetObject

namespace dcclite::broker
{
	static bool CheckIfServiceIsIgnorable(const rapidjson::Value &data)
	{
		auto ignoreServiceFlag = data.FindMember("ignoreOnLoadFailure");
		return ((ignoreServiceFlag != data.MemberEnd()) && (ignoreServiceFlag->value.GetBool()));		
	}

	static std::unique_ptr<Service> CreateBrokerService(Broker &broker, const rapidjson::Value &data, const Project &project)
	{
		const char *className = data["class"].GetString();
		RName name{ data["name"].GetString() };

		dcclite::Log::Info("[Broker] [CreateBrokerService] Creating DccLite Service: {}", name);

		try
		{
			if (strcmp(className, "Bonjour") == 0)
			{
				return BonjourService::Create(name, broker, project);
			}
			if (strcmp(className, "DccLiteService") == 0)
			{
				return std::make_unique<DccLiteService>(name, broker, data, project);
			}
			else if (strcmp(className, "DccppService") == 0)
			{
				return DccppService::Create(name, broker, data, project);
			}
			else if (strcmp(className, "DispatcherService") == 0)
			{
				return DispatcherService::Create(name, broker, data, project);
			}
			else if (strcmp(className, "LoconetService") == 0)
			{
				return LoconetService::Create(name, broker, data, project);
			}
			else if (strcmp(className, "Terminal") == 0)
			{
				return std::make_unique<TerminalService>(name, broker, data, project);
			}
			else if (strcmp(className, "ThrottleService") == 0)
			{
				return ThrottleService::Create(name, broker, data, project);
			}

			throw std::runtime_error(fmt::format("[Broker] [CreateBrokerService] error: unknown service type {}", className));
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

	Broker::Broker(dcclite::fs::path projectPath) :
		m_clRoot(RName{ "root" }),
		m_clProject(std::move(projectPath))
	{		
		{
			auto cmdHost = std::make_unique<TerminalCmdHost>();
			m_pclTerminalCmdHost = cmdHost.get();

			m_clRoot.AddChild(std::move(cmdHost));
		}

		using namespace dcclite;

		m_pServices = static_cast<FolderObject *>(m_clRoot.AddChild(
			std::make_unique<FolderObject>(SpecialFolders::GetName(SpecialFolders::Folders::ServicesId)))
		);				

		this->LoadConfig();

		ScriptSystem::Start(*this, m_clProject);

		//Start after load, so project name is already loaded
		ZeroConfSystem::Start(m_clProject.GetName());
	}

	void Broker::LoadConfig()
	{
		const auto configFileName = m_clProject.GetFilePath("broker.config.json");
		std::ifstream configFile(configFileName);

		if (!configFile)
		{
			throw std::runtime_error(fmt::format("[Broker] [LoadConfig] error: cannot open config file {}", configFileName.string()));
		}

		dcclite::Log::Debug("[Broker] [LoadConfig] Loaded config {}", configFileName.string());

		rapidjson::IStreamWrapper isw(configFile);
		rapidjson::Document data;
		data.ParseStream(isw);
		
		m_clProject.SetName(data["name"].GetString());

		const auto &services = data["services"];

		if (!services.IsArray())
		{
			throw std::runtime_error("[Broker] [LoadConfig] error: invalid config, expected services array");
		}

		auto bonjourSetting = data.FindMember("bonjourService");
		if ((bonjourSetting != data.MemberEnd()) && (bonjourSetting->value.GetBool()))
			m_pServices->AddChild(BonjourService::Create(RName{ BONJOUR_SERVICE_NAME }, *this, m_clProject));

		dcclite::Log::Debug("[Broker] [LoadConfig] Processing config services array entries: {}", services.Size());

		auto servicesDataArray = services.GetArray();

		std::vector<const rapidjson::Value *> pendingServices;		

		for (auto &serviceData : servicesDataArray)
		{
			auto requiresData = serviceData.FindMember("requires");
			if (requiresData == serviceData.MemberEnd())
			{
				std::unique_ptr<Service> service{ CreateBrokerService(*this, serviceData, m_clProject)};

				if (!service)
					continue;

				m_pServices->AddChild(std::move(service));
			}
			else
			{								
				pendingServices.push_back(&serviceData);
			}
		}

		for (auto &serviceData : pendingServices)
		{
			std::unique_ptr<Service> service{ CreateBrokerService(*this, *serviceData, m_clProject)};
			
			if (!service)
				continue;			

			m_pServices->AddChild(std::move(service));
		}		
	}

	Service &Broker::ResolveRequirement(const char *requirement)
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
			auto enumerator = m_pServices->GetEnumerator();
			while (enumerator.MoveNext())
			{
				auto obj = enumerator.GetCurrent();
				if (strcmp(reqName, obj->GetTypeName()) == 0)
					return *static_cast<Service *>(obj);
			}

			throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Requested service of type {} not found", requirement));
		}
		
		throw std::invalid_argument(fmt::format("[Broker::ResolveRequirement] Sybtax error parsing requirement {} ", requirement));
	}

	Service *Broker::TryFindService(RName name)
	{
		return static_cast<Service *>(m_pServices->TryGetChild(name));
	}
}

