#include "Brooker.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include <boost/log/trivial.hpp>

#include "json.hpp"

#include "DccLiteService.h"

using json = nlohmann::json;

static std::unique_ptr<Service> CreateService(const json &obj)
{
	std::string className = obj["class"];
	std::string name = obj["name"].get<std::string>();

	std::unique_ptr<Service> output;

	BOOST_LOG_TRIVIAL(info) << "Creating DccLite Service: " << name;

	if (!ServiceClass::TryProduce(output, className.c_str(), name))
	{
		std::stringstream stream;

		stream << "error: unknown service type " << className;
		throw std::runtime_error(stream.str());
	}

	return output;
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

	data << configFile;	

	const auto &services = data["services"];

	if (!services.is_array())
	{
		throw std::runtime_error("error: invalid config, expected services array");
	}

	for (unsigned i = 0; i < services.size(); ++i)
	{
		auto service = CreateService(services[i]);

		m_mapServices.insert(std::make_pair(service->GetName(), std::move(service)));
	}
}
