#include "DccLiteService.h"

#include <spdlog/spdlog.h>

#include <Log.h>

#include "Device.h"

static ServiceClass dccLiteService("DccLite", 
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<DccLiteService>(serviceClass, name, params); }
);

DccLiteService::DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) :
	Service(serviceClass, name, params)
{
	m_pDecoders = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("decoders")));
	m_pAddresses = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("addresses")));
	m_pDevices = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("devices")));	

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

		m_pDevices->AddChild(std::make_unique<Device>(nodeName, *this, device));
	}
}

DccLiteService::~DccLiteService()
{
	//empty
}

Decoder &DccLiteService::Create(
	const std::string &className,
	Decoder::Address address,
	const std::string &name,
	const nlohmann::json &params
)
{
	auto decoder = Decoder::Class::TryProduce(className.c_str(), address, name, *this, params);
	if (!decoder)
	{
		std::stringstream stream;

		stream << "error: failed to instantiate decoder " << address << " named " << name;

		throw std::runtime_error(stream.str());
	}

	auto pDecoder = decoder.get();	

	m_pDecoders->AddChild(std::move(decoder));
	m_pAddresses->AddChild(std::make_unique<dcclite::Shortcut>(pDecoder->GetAddress().ToString(), *pDecoder));

	return *pDecoder;
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

	dcclite::Log::Info("[DccLiteService::Update] got data");	
}

