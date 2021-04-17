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

#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <spdlog/logger.h>

#include "Log.h"

#include "TerminalCmd.h"
#include "TerminalService.h"
#include "SpecialFolders.h"

#include "DccLiteService.h"
#include "DccppService.h"

//fucking header leak
#undef GetObject

namespace dcclite::broker
{
	static std::unique_ptr<Service> CreateBrokerService(Broker &broker, const rapidjson::Value &data, const Project &project)
	{
		const char *className = data["class"].GetString();
		const char *name = data["name"].GetString();

		dcclite::Log::Info("Creating DccLite Service: {}", name);

		if (strcmp(className, "DccLite") == 0)
		{
			return std::make_unique<DccLiteService>(name, broker, data, project);
		}
		else if (strcmp(className, "DccppService") == 0)
		{
			return DccppService::Create(name, broker, data, project);
		}
		else if (strcmp(className, "Terminal") == 0)
		{
			return std::make_unique<TerminalService>(name, broker, data, project);
		}

		throw std::runtime_error(fmt::format("error: unknown service type {}", className));
	}

	Broker::Broker(dcclite::fs::path projectPath) :
		m_clRoot("root"),
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
	}



	void Broker::LoadConfig()
	{
		const auto configFileName = m_clProject.GetFilePath("broker.config.json");
		std::ifstream configFile(configFileName);

		if (!configFile)
		{
			throw std::runtime_error(fmt::format("error: cannot open config file {}", configFileName.string()));
		}

		dcclite::Log::Debug("Loaded config {}", configFileName.string());

		rapidjson::IStreamWrapper isw(configFile);
		rapidjson::Document data;
		data.ParseStream(isw);

		m_clProject.SetName(data["name"].GetString());

		const auto &services = data["services"];

		if (!services.IsArray())
		{
			throw std::runtime_error("error: invalid config, expected services array");
		}

		dcclite::Log::Debug("Processing config services {}", services.Size());

		for (auto &serviceData : services.GetArray())
		{
			auto service = CreateBrokerService(*this, serviceData, m_clProject);

			m_pServices->AddChild(std::move(service));
		}

		auto enumerator = m_pServices->GetEnumerator();

		while (enumerator.MoveNext())
		{
			enumerator.TryGetCurrent<Service>()->Initialize();
		}
	}

	void Broker::Update(const dcclite::Clock &clock)
	{
		auto enumerator = m_pServices->GetEnumerator();

		while (enumerator.MoveNext())
		{
			enumerator.TryGetCurrent<Service>()->Update(clock);
		}
	}

	Service *Broker::TryFindService(std::string_view name)
	{
		return static_cast<Service *>(m_pServices->TryGetChild(name));
	}
}

