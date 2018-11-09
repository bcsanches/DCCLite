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

static std::unique_ptr<Service> CreateService(const json &obj)
{
	std::string className = obj["class"];
	std::string name = obj["name"].get<std::string>();	

	dcclite::Log::Info("Creating DccLite Service: {}", name);
	
	if (auto output = ServiceClass::TryProduce(className.c_str(), name, obj))
	{
		return output;	
	}
	
	throw std::runtime_error(fmt::format("error: unknown service type {}", className));
}

Brooker::Brooker():
	m_clRoot("root")
{
	//empty
}

void Brooker::LoadConfig(const char *configFileName)
{
	std::ifstream configFile(configFileName);

	if (!configFile)
	{
		throw std::runtime_error("error: cannot open config file");		
	}

	dcclite::Log::Debug("Loaded config {}", configFileName);

	json data;

	configFile >> data;

	const auto &services = data["services"];

	if (!services.is_array())
	{
		throw std::runtime_error("error: invalid config, expected services array");
	}

	dcclite::Log::Debug("Processing config services {}", services.size());

	for(auto &serviceData : services)	
	{
		auto service = CreateService(serviceData);

		m_clRoot.AddChild(std::move(service));
	}
}

void Brooker::Update()
{
	auto enumerator = m_clRoot.GetEnumerator();

	while (enumerator.MoveNext())
	{
		enumerator.TryGetCurrent<Service>()->Update();
	}	
}
