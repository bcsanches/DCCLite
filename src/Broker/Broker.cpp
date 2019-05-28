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
#include "SpecialFolders.h"

#include "DccLiteService.h"

//fucking header leak
#undef GetObject

static std::unique_ptr<Service> CreateService(Broker &broker, const rapidjson::Value &data, const Project &project)
{
	const char *className = data["class"].GetString();
	const char *name = data["name"].GetString();

	dcclite::Log::Info("Creating DccLite Service: {}", name);
	
	if (auto output = ServiceClass::TryProduce(className, name,  broker, data, project))
	{
		return output;	
	}
	
	throw std::runtime_error(fmt::format("error: unknown service type {}", className));
}

Broker::Broker(std::filesystem::path projectPath):
	m_clRoot("root"),
	m_clProject(std::move(projectPath))
{	
	m_clRoot.AddChild(std::make_unique<TerminalCmdHost>());

	using namespace dcclite;

	m_pServices = static_cast<FolderObject*>(m_clRoot.AddChild(
		std::make_unique<FolderObject>(SpecialFolders::GetName(SpecialFolders::ServicesFolderId)))
	);

	this->LoadConfig();
}

void Broker::LoadConfig()
{
	const auto configFileName = m_clProject.GetFilePath("broker.config.json");
	std::ifstream configFile(configFileName);

	if (!configFile)
	{
		throw std::runtime_error("error: cannot open config file");		
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
	
	for(auto &serviceData : services.GetArray())	
	{		
		auto service = CreateService(*this, serviceData, m_clProject);			

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

Service* Broker::TryFindService(std::string_view name)
{
	return static_cast<Service *>(m_pServices->TryGetChild(name));
}
