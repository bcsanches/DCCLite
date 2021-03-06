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

#include <exception>
#include <Log.h>

#include "NetworkDevice.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "LocationManager.h"
#include "OutputDecoder.h"
#include "Packet.h"
#include "SensorDecoder.h"
#include "SignalDecoder.h"
#include "SimpleOutputDecoder.h"
#include "TurnoutDecoder.h"
#include "VirtualDevice.h"

namespace dcclite::broker
{
	std::unique_ptr<Decoder> TryCreateDecoder(const std::string &className, DccAddress address, std::string name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params)
	{
		if (className.compare("Output") == 0)
			return std::make_unique<SimpleOutputDecoder>(address, std::move(name), owner, dev, params);
		else if (className.compare("Sensor") == 0)
			return std::make_unique<SensorDecoder>(address, std::move(name), owner, dev, params);
		else if (className.compare("ServoTurnout") == 0)
			return std::make_unique<ServoTurnoutDecoder>(address, std::move(name), owner, dev, params);
		else if (className.compare("VirtualSignal") == 0)
			return std::make_unique<SignalDecoder>(address, std::move(name), owner, dev, params);
		else
			return std::unique_ptr<Decoder>();
	}


	DccLiteService::DccLiteService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)	
	{
		m_pDecoders = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("decoders")));
		m_pAddresses = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("addresses")));
		m_pDevices = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("devices")));	
		m_pSessions = static_cast<FolderObject*>(this->AddChild(std::make_unique<FolderObject>("sessions")));

		m_pLocations = static_cast<LocationManager *>(this->AddChild(std::make_unique<LocationManager>("locations", params)));

		auto port = params["port"].GetInt();
	
		if (!m_clSocket.Open(port, dcclite::Socket::Type::DATAGRAM))
		{
			throw std::runtime_error("[DccLiteService] error: cannot open socket");
		}

		dcclite::Log::Info("[DccLiteService::DccLiteService] Listening on port {}", port);

		const rapidjson::Value &devicesData = params["devices"];

		if (!devicesData.IsArray())
			throw std::runtime_error(fmt::format("error: invalid config {}, expected devices array inside DccLiteService", name));

		try
		{
			for (auto &device : devicesData.GetArray())
			{
				auto nodeName = device["name"].GetString();
				auto className = device["class"].GetString();

				if (strcmp(className, "Virtual"))
					m_pDevices->AddChild(std::make_unique<NetworkDevice>(nodeName, *static_cast<IDccLite_DeviceServices *>(this), device, project));
				else
					m_pDevices->AddChild(std::make_unique<VirtualDevice>(nodeName, *static_cast<IDccLite_DeviceServices *>(this), device, project));
			}
		}
		catch (std::exception &)
		{
			//cleanup before exception blew up everything, otherwise devices get destroyed after us are gone and system goes crazy
			this->RemoveChild(m_pDevices->GetName());		

			throw;
		}
	
	}

	DccLiteService::~DccLiteService()
	{
		//Destroy devices, so they clean everything
		this->RemoveChild(m_pDevices->GetName());
	}

	void DccLiteService::Device_DestroyDecoder(Decoder &dec)
	{
		//send mesage before we destroy it
		this->NotifyItemDestroyed(dec);

		auto addressName = dec.GetAddress().ToString();
		this->NotifyItemDestroyed(*(m_pAddresses->TryGetChild(addressName)));

		m_pLocations->UnregisterDecoder(dec);

		m_pAddresses->RemoveChild(addressName);
		m_pDecoders->RemoveChild(dec.GetName());	
	}

	Decoder &DccLiteService::Device_CreateDecoder(
		IDevice_DecoderServices &dev,
		const std::string &className,
		DccAddress address,
		const std::string &name,
		const rapidjson::Value &params
	)
	{
		auto decoder = TryCreateDecoder(className.c_str(), address, name, *this, dev, params);	
		if (!decoder)
		{				
			throw std::runtime_error(fmt::format("[DccLiteService::Device_CreateDecoder] Error: failed to instantiate decoder {} - {} [{}]", name, address, className));
		}

		auto pDecoder = decoder.get();	

		m_pDecoders->AddChild(std::move(decoder));

		try
		{
			auto addressShortcut = m_pAddresses->AddChild(
				std::make_unique<dcclite::Shortcut>(
					pDecoder->GetAddress().ToString(),
					*pDecoder
				)
			);

			this->NotifyItemCreated(*pDecoder);
			this->NotifyItemCreated(*addressShortcut);
		}
		catch (...)
		{
			//something bad happenned, cleanup to keep a consistent state
			m_pDecoders->RemoveChild(pDecoder->GetName());

			//blow up
			throw;
		}		

		m_pLocations->RegisterDecoder(*pDecoder);

		return *pDecoder;
	}

	void DccLiteService::Device_NotifyInternalItemCreated(const dcclite::IObject &item) const
	{
		this->NotifyItemCreated(item);
	}

	void DccLiteService::Device_NotifyInternalItemDestroyed(const dcclite::IObject &item) const
	{
		this->NotifyItemDestroyed(item);
	}

	Device *DccLiteService::TryFindDeviceByName(std::string_view name)
	{	
		return static_cast<Device *>(m_pDevices->TryGetChild(name));
	}

	NetworkDevice *DccLiteService::TryFindDeviceSession(const dcclite::Guid &guid)
	{
		return static_cast<NetworkDevice *>(m_pSessions->TryResolveChild(dcclite::GuidToString(guid)));
	}

	void DccLiteService::OnNet_Discovery(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet)
	{
		dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] received discovery from {}, sending reply", this->GetName(), senderAddress);

		dcclite::Packet pkt;

		//just send a blank packet, so they know we are here
		dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::DISCOVERY, dcclite::Guid{}, dcclite::Guid{} };
	
		this->Device_SendPacket(senderAddress, pkt);
	}

	void DccLiteService::OnNet_Hello(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet)
	{
		auto remoteSessionToken = packet.ReadGuid();
		auto remoteConfigToken = packet.ReadGuid();
		
		dcclite::PacketReader reader(packet);	

		char name[256];
		reader.ReadStr(name, sizeof(name));

		const auto procotolVersion = packet.Read<std::uint16_t>();
		if (procotolVersion != dcclite::PROTOCOL_VERSION)
		{
			dcclite::Log::Error("[{}::DccLiteService::OnNet_Hello] Hello from {} - {} with invalid protocol version {}, expected {}, ignoring", 
				this->GetName(), 
				name, 
				senderAddress,
				procotolVersion,
				dcclite::PROTOCOL_VERSION
			);

			return;
		}

		dcclite::Log::Info("[{}::DccLiteService::OnNet_Hello] received hello from {}, starting handshake", this->GetName(), name);

		//lookup device
		auto dev = this->TryFindDeviceByName(name);	
		NetworkDevice *netDevice;
		if (dev == nullptr)
		{
			//no device, create a temp one
			dcclite::Log::Warn("[{}::DccLiteService::OnNet_Hello] {} is not on config", this->GetName(), name);

			netDevice = static_cast<NetworkDevice*>(m_pDevices->AddChild(
				std::make_unique<NetworkDevice>(
					name, 
					*static_cast<IDccLite_DeviceServices *>(this), 
					m_rclProject
				)
			));
		}	
		else
		{
			netDevice = dynamic_cast<NetworkDevice *>(dev);
			if (netDevice == nullptr)
			{
				dcclite::Log::Error("[{}::DccLiteService::OnNet_Hello] {} is not a network device, cannot accept connection. Please check config.", this->GetName(), name);

				return;
			}
		}

		netDevice->AcceptConnection(clock.Ticks(), senderAddress, remoteSessionToken, remoteConfigToken);
	}

	NetworkDevice *DccLiteService::TryFindPacketDestination(dcclite::Packet &packet)
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

	void DccLiteService::OnNet_Packet(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet, const dcclite::MsgTypes msgType)
	{
		auto dev = TryFindPacketDestination(packet);
		if (!dev)
		{
			dcclite::Log::Warn("[{}::DccLiteService::OnNet_Packet] Received packet from unkown device", this->GetName());

			return;
		}

		dcclite::Guid configToken = packet.ReadGuid();

		dev->OnPacket(packet, clock.Ticks(), msgType, senderAddress, configToken);
	}


	void DccLiteService::Device_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet &packet)
	{
		if (!m_clSocket.Send(destination, packet.GetData(), packet.GetSize()))
		{
			dcclite::Log::Error("[{}::DccLiteService::Device_SendPacket] Failed to send packet to {}", this->GetName(), destination);
		}
	}

	void DccLiteService::Device_RegisterSession(NetworkDevice &dev, const dcclite::Guid &sessionToken)
	{
		auto session = m_pSessions->AddChild(std::make_unique<dcclite::Shortcut>(dcclite::GuidToString(sessionToken), dev));

		this->NotifyItemCreated(*session);

		this->NotifyItemChanged(dev);		
	}

	void DccLiteService::Device_UnregisterSession(NetworkDevice& dev, const dcclite::Guid &sessionToken)
	{	
		auto session = m_pSessions->RemoveChild(dcclite::GuidToString(sessionToken));	
		
		this->NotifyItemChanged(dev);
					
		this->NotifyItemDestroyed(*session);
	}

	Decoder* DccLiteService::TryFindDecoder(const DccAddress address) const
	{
		return this->TryFindDecoder(address.ToString());
	}

	Decoder *DccLiteService::TryFindDecoder(std::string_view id) const
	{
		auto *decoder = m_pAddresses->TryResolveChild(id);

		return static_cast<Decoder *>(decoder ? decoder : m_pDecoders->TryResolveChild(id));
	}

	std::vector<SimpleOutputDecoder *> DccLiteService::FindAllSimpleOutputDecoders()
	{
		std::vector<SimpleOutputDecoder*> vecDecoders;

		auto enumerator = m_pDecoders->GetEnumerator();

		while (enumerator.MoveNext())
		{
			auto decoder = dynamic_cast<SimpleOutputDecoder *>(enumerator.TryGetCurrent());

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

	std::vector<TurnoutDecoder *> DccLiteService::FindAllTurnoutDecoders()
	{
		std::vector<TurnoutDecoder *> vecDecoders;

		auto enumerator = m_pDecoders->GetEnumerator();

		while (enumerator.MoveNext())
		{
			auto decoder = dynamic_cast<TurnoutDecoder *>(enumerator.TryGetCurrent());

			if (!decoder)
				continue;

			vecDecoders.push_back(decoder);
		}

		return vecDecoders;
	}

	void DccLiteService::Decoder_OnStateChanged(Decoder& decoder)
	{
		this->NotifyItemChanged(decoder);		
	}

	void DccLiteService::Update(const dcclite::Clock &clock)
	{
		std::uint8_t data[2048];

		dcclite::NetworkAddress sender;

		for (;;)
		{
			auto [status, size] = m_clSocket.Receive(sender, data, sizeof(data));

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

				default:
					this->OnNet_Packet(clock, sender, pkt, msgType);
					break;
			}
		}

		auto enumerator = this->m_pDevices->GetEnumerator();
		while (enumerator.MoveNext())
		{
			auto dev = enumerator.TryGetCurrent<Device>();

			dev->Update(clock);
		}
	}
}

