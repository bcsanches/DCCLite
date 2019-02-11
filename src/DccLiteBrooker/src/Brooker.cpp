#include "Brooker.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <fmt/format.h>
#include <spdlog/logger.h>

#include "Log.h"

#include "json.hpp"

#include "DccLiteService.h"

using json = nlohmann::json;

static std::unique_ptr<Service> CreateService(const json &obj, const Project &project)
{
	std::string className = obj["class"];
	std::string name = obj["name"].get<std::string>();	

	dcclite::Log::Info("Creating DccLite Service: {}", name);
	
	if (auto output = ServiceClass::TryProduce(className.c_str(), name, obj, project))
	{
		return output;	
	}
	
	throw std::runtime_error(fmt::format("error: unknown service type {}", className));
}

Brooker::Brooker(std::filesystem::path projectPath):
	m_clRoot("root"),
	m_clProject(std::move(projectPath))
{
	this->LoadConfig();
}

void Brooker::LoadConfig()
{
	const auto configFileName = m_clProject.GetFilePath("brooker.config.json");
	std::ifstream configFile(configFileName);

	if (!configFile)
	{
		throw std::runtime_error("error: cannot open config file");		
	}

	dcclite::Log::Debug("Loaded config {}", configFileName.string());

	json data;

	configFile >> data;

	m_clProject.SetName(data["name"].get<std::string>());

	const auto &services = data["services"];

	if (!services.is_array())
	{
		throw std::runtime_error("error: invalid config, expected services array");
	}

	dcclite::Log::Debug("Processing config services {}", services.size());

	for(auto &serviceData : services)	
	{
		auto service = CreateService(serviceData, m_clProject);

		m_clRoot.AddChild(std::move(service));
	}
}

void Brooker::Update(const dcclite::Clock &clock)
{
	auto enumerator = m_clRoot.GetEnumerator();

	while (enumerator.MoveNext())
	{
		enumerator.TryGetCurrent<Service>()->Update(clock);
	}	
}
