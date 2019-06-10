// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DccLiteService.h"

#include <spdlog/spdlog.h>

#include <Log.h>

#include "Device.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "OutputDecoder.h"
#include "SensorDecoder.h"
#include "Packet.h"

static ServiceClass dccLiteService("DccLite", 
	[](const ServiceClass &serviceClass, const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) ->
	std::unique_ptr<Service> { return std::make_unique<DccLiteService>(serviceClass, name, broker, params, project); }
);

DccLiteService::DccLiteService(const ServiceClass &serviceClass, const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
	Service(serviceClass, name, broker, params, project)	
{
	m_pDecoders = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("decoders")));
	m_pAddresses = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("addresses")));
	m_pDevices = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("devices")));	
	m_pSessions = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("sessions")));

	auto port = params["port"].GetInt();

	if (!m_clSocket.Open(port, dcclite::Socket::Type::DATAGRAM))
	{
		throw std::runtime_error("[DccLiteService] error: cannot open socket");
	}

	const rapidjson::Value &devicesData = params["devices"];

	if (!devicesData.IsArray())
		throw std::runtime_error("error: invalid config, expected devices array inside DccLiteService");

	for (auto &device : devicesData.GetArray())
	{
		auto nodeName = device["name"].GetString();

		m_pDevices->AddChild(std::make_unique<Device>(nodeName, *this, device, project));
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
	const rapidjson::Value &params
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

Device *DccLiteService::TryFindDeviceSession(const dcclite::Guid &guid)
{
	return static_cast<Device *>(m_pSessions->TryResolveChild(dcclite::GuidToString(guid)));
}

void DccLiteService::Update(const dcclite::Clock &clock)
{
	std::uint8_t data[2048];

	dcclite::Address sender;	

	for (;;)
	{	
		auto[status, size] = m_clSocket.Receive(sender, data, sizeof(data));

		if (status != dcclite::Socket::Status::OK)
		{
			break;
		}

		//dcclite::Log::Info("[DccLiteService::Update] got data");

		if (size > dcclite::PACKET_MAX_SIZE)
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
			case dcclite::MsgTypes::DISCOVERY:
				this->OnNet_Discovery(clock, sender, pkt);
				break;

			case dcclite::MsgTypes::HELLO:			
				this->OnNet_Hello(clock, sender, pkt);
				break;

			case dcclite::MsgTypes::MSG_PING:
				this->OnNet_Ping(clock, sender, pkt);
				break;

			case dcclite::MsgTypes::CONFIG_ACK:
				this->OnNet_ConfigAck(clock, sender, pkt);
				break;

			case dcclite::MsgTypes::CONFIG_FINISHED:
				this->OnNet_ConfigFinished(clock, sender, pkt);
				break;

			case dcclite::MsgTypes::STATE:
				this->OnNet_State(clock, sender, pkt);
				break;

			default:
				dcclite::Log::Error(" msg type: {}", static_cast<uint8_t>(msgType));
				return;
		}
	}	

#if 1
	auto enumerator = this->m_pDevices->GetEnumerator();
	while (enumerator.MoveNext())
	{
		auto dev = enumerator.TryGetCurrent<Device>();

		dev->Update(clock);
	}
#endif
}

void DccLiteService::OnNet_Discovery(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] received discovery from {}, sending reply", this->GetName(), senderAddress);

	dcclite::Packet pkt;

	//just send a blank packet, so they know we are here
	this->Device_PreparePacket(pkt, dcclite::MsgTypes::DISCOVERY, dcclite::Guid{}, dcclite::Guid{});
	this->Device_SendPacket(senderAddress, pkt);
}

void DccLiteService::OnNet_Hello(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
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

	dev->AcceptConnection(clock.Now(), senderAddress, remoteSessionToken, remoteConfigToken);	
}

Device *DccLiteService::TryFindPacketDestination(dcclite::Packet &packet)
{	
	dcclite::Guid sessionToken = packet.ReadGuid();	

	auto dev = this->TryFindDeviceSession(sessionToken);
	if (dev == nullptr)
	{
		dcclite::Log::Warn("[{}::DccLiteService::TryFindPacketDestination] Received packet from unknown session", this->GetName());

		return nullptr;
	}

	return dev;
}

void DccLiteService::OnNet_Ping(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
{	
	auto dev = TryFindPacketDestination(packet);
	if (!dev)
	{
		dcclite::Log::Warn("[{}::DccLiteService::OnNet_Hello] Received ping from unkown device", this->GetName());

		return;
	}		

	dcclite::Guid configToken = packet.ReadGuid();

	dev->OnPacket_Ping(packet, clock.Now(), senderAddress, configToken);
}

void DccLiteService::OnNet_ConfigAck(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	//dcclite::Log::Trace("[{}::DccLiteService::OnNet_Hello] Received ping, sending pong...", this->GetName());	

	auto dev = TryFindPacketDestination(packet);
	if (!dev)
		return;

	dcclite::Guid configToken = packet.ReadGuid();

	dev->OnPacket_ConfigAck(packet, clock.Now(), senderAddress);
}

void DccLiteService::OnNet_ConfigFinished(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	auto dev = TryFindPacketDestination(packet);
	if (!dev)
		return;

	dcclite::Guid configToken = packet.ReadGuid();

	dev->OnPacket_ConfigFinished(packet, clock.Now(), senderAddress);

}

void DccLiteService::OnNet_State(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet)
{
	auto dev = TryFindPacketDestination(packet);
	if (!dev)
		return;

	dcclite::Guid configToken = packet.ReadGuid();

	dev->OnPacket_State(packet, clock.Now(), senderAddress, configToken);
}


void DccLiteService::Device_PreparePacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, const dcclite::Guid &sessionToken, const dcclite::Guid &configToken)
{
	dcclite::PacketBuilder builder{ packet, msgType, sessionToken, configToken };
}

void DccLiteService::Device_SendPacket(const dcclite::Address destination, const dcclite::Packet &packet)
{
	if (!m_clSocket.Send(destination, packet.GetData(), packet.GetSize()))
	{
		dcclite::Log::Error("[{}::DccLiteService::Device_SendPacket] Failed to send packet to {}", this->GetName(), destination);
	}
}

void DccLiteService::Device_RegisterSession(Device &dev, const dcclite::Guid &sessionToken)
{
	m_pSessions->AddChild(std::make_unique<dcclite::Shortcut>(dcclite::GuidToString(sessionToken), dev));
}

void DccLiteService::Device_UnregisterSession(const dcclite::Guid &sessionToken)
{	
	m_pSessions->RemoveChild(dcclite::GuidToString(sessionToken));
}

Decoder* DccLiteService::TryFindDecoder(const Decoder::Address address) const
{
	return this->TryFindDecoder(address.ToString());
}

Decoder *DccLiteService::TryFindDecoder(std::string_view id) const
{
	auto *decoder = m_pAddresses->TryResolveChild(id);

	return static_cast<Decoder *>(decoder ? decoder : m_pDecoders->TryResolveChild(id));
}

std::vector<OutputDecoder*> DccLiteService::FindAllOutputDecoders()
{
	std::vector<OutputDecoder*> vecDecoders;

	auto enumerator = m_pDecoders->GetEnumerator();

	while (enumerator.MoveNext())
	{
		auto decoder = dynamic_cast<OutputDecoder *>(enumerator.TryGetCurrent());

		if (!decoder)
			continue;

		vecDecoders.push_back(decoder);
	}

	return vecDecoders;
}

std::vector<SensorDecoder*> DccLiteService::FindAllSensorDecoders()
{
	std::vector<SensorDecoder*> vecDecoders;

	auto enumerator = m_pDecoders->GetEnumerator();

	while (enumerator.MoveNext())
	{
		auto decoder = dynamic_cast<SensorDecoder *>(enumerator.TryGetCurrent());

		if (!decoder)
			continue;

		vecDecoders.push_back(decoder);
	}

	return vecDecoders;
}
