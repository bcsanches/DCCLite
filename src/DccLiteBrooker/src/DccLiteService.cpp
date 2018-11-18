#include "DccLiteService.h"

#include <spdlog/spdlog.h>

#include <Log.h>

#include "Device.h"
#include "GuidUtils.h"
#include "FmtUtils.h"
#include "Packet.h"

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

Device *DccLiteService::TryFindDeviceByName(std::string_view name)
{	
	return static_cast<Device *>(m_pDevices->TryGetChild(name));
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

	if (size >= dcclite::PACKET_MAX_SIZE)
	{
		dcclite::Log::Error("[DccLiteService::Update] packet size too big, truncating");

		size = dcclite::PACKET_MAX_SIZE;
	}

	dcclite::Packet pkt{ data, static_cast<uint8_t>(size) };

	//dcclite::PacketReader reader{ pkt };	

	if (pkt.Read<uint32_t>() != dcclite::PACKET_ID)
	{
		dcclite::Log::Warn("[DccLiteService::Update] Invalid packet id");

		return;
	}

	auto msgType = static_cast<dcclite::MsgTypes>(pkt.Read<uint8_t>());

	switch (msgType)
	{
		case dcclite::MsgTypes::HELLO:			
			this->OnNet_Hello(sender, pkt);
			break;

		case dcclite::MsgTypes::PING:
			this->OnNet_Ping(sender, pkt);
			break;

		default:
			dcclite::Log::Error("Invalid msg type: {}", static_cast<uint8_t>(msgType));
			return;
	}
}

void DccLiteService::SendPacket(const dcclite::Address &senderAddress, const dcclite::Packet &packet)
{
	if (!m_clSocket.Send(senderAddress, packet.GetData(), packet.GetSize()))
	{
		dcclite::Log::Error("[{}::DccLiteService::SendPacket] Failed to send answer to device {}", this->GetName(), senderAddress);
	}
}

void DccLiteService::OnNet_Hello(const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	auto remoteSessionToken = packet.ReadGuid();
	auto remoteConfigToken = packet.ReadGuid();

	char name[256];
	
	dcclite::PacketReader reader(packet);

	reader.ReadStr(name, sizeof(name));

	dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] received hello from {}, starting handshake", this->GetName(), name);

	//lookup device
	auto dev = this->TryFindDeviceByName(name);
	if (dev == nullptr)
	{
		//no device, create a temp one
		dcclite::Log::Warn("[{}::DccLiteService::OnNet_Hello] {} is not on config", this->GetName(), name);

		dev = static_cast<Device*>(m_pDevices->AddChild(std::make_unique<Device>(name, *this)));
	}	

	auto sessionToken = dcclite::GuidCreate();
	dev->SetSessionToken(sessionToken);

	auto configToken = dev->GetConfigToken();
	if (configToken.IsNull())
	{		
		configToken = dcclite::GuidCreate();
		dev->SetConfigToken(configToken);

		dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] Created config token {} for {}", this->GetName(), configToken, name);
	}
	
	dcclite::Packet response;

	if (remoteConfigToken != configToken)
	{
		dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] Started configuring {}", this->GetName(), name);
		dev->SetStatus(Device::Status::CONFIGURING);
		
		{
			dcclite::PacketBuilder responseBuilder{ response, dcclite::MsgTypes::CONFIG_START, sessionToken, dcclite::Guid{} };
			SendPacket(senderAddress, response);
		}
		
		{
			response.Reset();

			dcclite::PacketBuilder responseBuilder{ response, dcclite::MsgTypes::CONFIG_FINISHED, sessionToken, configToken };
			SendPacket(senderAddress, response);
		}

		dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] {} configured", this->GetName(), name);
	}
	else
	{
		dev->SetStatus(Device::Status::ONLINE);

		dcclite::PacketBuilder responseBuilder{ response, dcclite::MsgTypes::ACCEPTED, sessionToken, configToken };

		SendPacket(senderAddress, response);
	}

	dev->SetRemoteAddress(senderAddress);	
}

void DccLiteService::OnNet_Ping(const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	dcclite::Log::Trace("[{}::DccLiteService::OnNet_Hello] Received ping, sending pong...", this->GetName());

	auto remoteSessionToken = packet.ReadGuid();
	auto remoteConfigToken = packet.ReadGuid();

	dcclite::Packet pkt;

	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::PONG, remoteSessionToken, remoteConfigToken };

	if (!m_clSocket.Send(senderAddress, pkt.GetData(), pkt.GetSize()))
	{
		dcclite::Log::Error("[{}::DccLiteService::OnNet_Ping] Failed to send answer to device {}", this->GetName(), senderAddress);
	}
}

