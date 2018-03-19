#include "Brooker.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <plog/Log.h>

#include "json.hpp"

#include "DccLiteService.h"

using json = nlohmann::json;

static std::unique_ptr<Service> CreateService(const json &obj)
{
	std::string className = obj["class"];
	std::string name = obj["name"].get<std::string>();	

	LOG_INFO << "Creating DccLite Service: " << name;
	
	if (auto output = ServiceClass::TryProduce(className.c_str(), name, obj))
	{
		return output;	
	}

	std::stringstream stream;

	stream << "error: unknown service type " << className;
	throw std::runtime_error(stream.str());
}

Brooker::Brooker()
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

	json data;

	configFile >> data;

	const auto &services = data["services"];

	if (!services.is_array())
	{
		throw std::runtime_error("error: invalid config, expected services array");
	}

	for(auto &serviceData : services)	
	{
		auto service = CreateService(serviceData);

		m_mapServices.insert(std::make_pair(service->GetName(), std::move(service)));
	}
}

void Brooker::Update()
{
	for (auto &it : m_mapServices)
	{
		it.second->Update();
	}
}
