#include "DccLiteService.h"

#include <plog/Log.h>

#include "Device.h"

static ServiceClass dccLiteService("DccLite", 
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<DccLiteService>(serviceClass, name, params); }
);

DccLiteService::DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) :
	Service(serviceClass, name, params)
{
	auto port = params["port"].get<int>();

	if (!m_clSocket.Open(port, dcclite::Socket::Type::DATAGRAM))
	{
		throw std::runtime_error("[DccLiteService] error: cannot open socket");
	}

	auto devicesData = params["devices"];

	if (!devicesData.is_array())
		throw std::runtime_error("error: invalid config, expected devices array inside DccLiteService");

	for (auto &device : devicesData)
	{
		auto nodeName = device["name"].get<std::string>();

		auto existingNodeIt = m_mapDevices.find(nodeName);
		if (existingNodeIt != m_mapDevices.end())
		{
			std::stringstream stream;

			stream << "error: device " << nodeName << " already exists on this service (" << this->GetName() << ").";

			throw std::runtime_error(stream.str());
		}		

		auto pair = m_mapDevices.insert(
			existingNodeIt,
			std::make_pair(
				nodeName,
				std::make_unique<Device>(nodeName, *this, device)
			)
		);
	}
}

DccLiteService::~DccLiteService()
{
	//empty
}

void DccLiteService::Update()
{
	std::uint8_t data[2048];

	dcclite::Address sender;	

	auto[status, size] = m_clSocket.Receive(sender, data, sizeof(data));

	if (status != dcclite::Socket::Status::OK)
	{
		return;
	}

	LOG_INFO << "[DccLiteService::Update] got data";
}

