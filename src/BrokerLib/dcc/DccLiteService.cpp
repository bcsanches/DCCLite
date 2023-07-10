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

#include <FmtUtils.h>
#include <GuidUtils.h>
#include <Packet.h>
#include <Util.h>

#include "NetworkDevice.h"
#include "LocationManager.h"
#include "OutputDecoder.h"
#include "QuadInverter.h"
#include "SensorDecoder.h"
#include "SignalDecoder.h"
#include "SimpleOutputDecoder.h"
#include "TurnoutDecoder.h"
#include "TurntableAutoInverterDecoder.h"
#include "VirtualDevice.h"

#include "../sys/EventHub.h"

using namespace std::chrono_literals;

namespace dcclite::broker
{
	std::unique_ptr<Decoder> TryCreateDecoder(const std::string &className, DccAddress address, std::string name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params)
	{
		//
		//Check the most common first....
		//
		if (className.compare("ServoTurnout") == 0)
			return std::make_unique<ServoTurnoutDecoder>(address, std::move(name), owner, dev, params);

		if (className.compare("Output") == 0)
			return std::make_unique<SimpleOutputDecoder>(address, std::move(name), owner, dev, params);

		if (className.compare("Sensor") == 0)
			return std::make_unique<SensorDecoder>(address, std::move(name), owner, dev, params);

		if (className.compare("VirtualSignal") == 0)
			return std::make_unique<SignalDecoder>(address, std::move(name), owner, dev, params);
		
		if (className.compare("QuadInverter") == 0)
			return std::make_unique<QuadInverter>(address, std::move(name), owner, dev, params);
		
		if (className.compare("TurntableAutoInverter") == 0)
			return std::make_unique<TurntableAutoInverterDecoder>(address, std::move(name), owner, dev, params);
					
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
	
		if (!m_clSocket.Open(port, dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error(fmt::format("[DccLiteService::{}] error: cannot open socket", name));
		}

		dcclite::Log::Info("[DccLiteService::{}] Listening on port {}", this->GetName(), port);

		const rapidjson::Value &devicesData = params["devices"];

		if (!devicesData.IsArray())
			throw std::runtime_error(fmt::format("[DccLiteService::{}] error: invalid config, expected devices array inside DccLiteService", name));

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
		
		m_clNetworkThread = std::thread{ [this] {this->NetworkThreadProc(); } };
		dcclite::SetThreadName(m_clNetworkThread, "DccLiteService::NetworkThread");
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
		auto decoder = TryCreateDecoder(className, address, name, *this, dev, params);	
		if (!decoder)
		{				
			throw std::runtime_error(fmt::format("[DccLiteService::{}] [Device_CreateDecoder] Error: failed to instantiate decoder {} - {} [{}]", name, address, className));
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

	void DccLiteService::Device_NotifyInternalItemCreated(dcclite::IObject &item) const
	{
		this->NotifyItemCreated(item);
	}

	void DccLiteService::Device_NotifyInternalItemDestroyed(dcclite::IObject &item) const
	{
		this->NotifyItemDestroyed(item);
	}

	void DccLiteService::Device_NotifyStateChange(NetworkDevice &device) const
	{
		this->NotifyItemChanged(device);
	}

	Device *DccLiteService::TryFindDeviceByName(std::string_view name)
	{	
		return static_cast<Device *>(m_pDevices->TryGetChild(name));
	}

	NetworkDevice *DccLiteService::TryFindDeviceSession(const dcclite::Guid &guid)
	{
		return static_cast<NetworkDevice *>(m_pSessions->TryResolveChild(dcclite::GuidToString(guid)));
	}	

	NetworkDevice *DccLiteService::TryFindPacketDestination(dcclite::Packet &packet)
	{	
		dcclite::Guid sessionToken = packet.ReadGuid();	

		auto dev = this->TryFindDeviceSession(sessionToken);
		if (dev == nullptr)
		{
			dcclite::Log::Warn("[DccLiteService::{}] [TryFindPacketDestination] Received packet from unknown session", this->GetName());

			return nullptr;
		}

		return dev;
	}

	void DccLiteService::Device_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet &packet)
	{
		if (!m_clSocket.Send(destination, packet.GetData(), packet.GetSize()))
		{
			dcclite::Log::Error("[DccLiteService::{}] [Device_SendPacket] Failed to send packet to {}", this->GetName(), destination);
		}
	}

	void DccLiteService::Device_RegisterSession(NetworkDevice &dev, const dcclite::Guid &sessionToken)
	{
		auto session = m_pSessions->AddChild(std::make_unique<dcclite::Shortcut>(dcclite::GuidToString(sessionToken), dev));

		this->NotifyItemCreated(*session);		
	}

	void DccLiteService::Device_UnregisterSession(NetworkDevice& dev, const dcclite::Guid &sessionToken)
	{	
		auto session = m_pSessions->RemoveChild(dcclite::GuidToString(sessionToken));				
					
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
			auto decoder = dynamic_cast<SimpleOutputDecoder *>(enumerator.GetCurrent());

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
			auto decoder = dynamic_cast<SensorDecoder *>(enumerator.GetCurrent());

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
			auto decoder = dynamic_cast<TurnoutDecoder *>(enumerator.GetCurrent());

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

	//
	//
	// Network Thread
	//
	//

	void DccLiteService::NetworkThread_OnDiscovery(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet)
	{
		dcclite::Log::Info("[DccLiteService::{}] [OnNet_Hello] received discovery from {}, sending reply", this->GetName(), senderAddress);

		dcclite::Packet pkt;

		//just send a blank packet, so they know we are here
		dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::DISCOVERY, dcclite::Guid{}, dcclite::Guid{} };

		this->Device_SendPacket(senderAddress, pkt);
	}

	class NetworkHelloEvent: public dcclite::broker::EventHub::IEvent
	{
		public:
			NetworkHelloEvent(DccLiteService &target, dcclite::NetworkAddress address, std::string_view deviceName, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken):
				IEvent(target),
				m_clAddress(address),
				m_clRemoteSessionToken{ remoteSessionToken },
				m_clRemoteConfigToken{ remoteConfigToken },
				m_strDeviceName{ deviceName }
			{
				//empty
			}

			void Fire() override
			{
				static_cast<DccLiteService &>(this->GetTarget()).OnNetEvent_Hello(m_clAddress, m_strDeviceName, m_clRemoteSessionToken, m_clRemoteConfigToken);
			}

		private:
			const dcclite::NetworkAddress m_clAddress;

			const dcclite::Guid m_clRemoteSessionToken;
			const dcclite::Guid m_clRemoteConfigToken;

			std::string m_strDeviceName;
	};

	void DccLiteService::NetworkThread_OnNetHello(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet)
	{
		auto remoteSessionToken = packet.ReadGuid();
		auto remoteConfigToken = packet.ReadGuid();

		dcclite::PacketReader reader(packet);

		char name[256];
		reader.ReadStr(name, sizeof(name));

		const auto procotolVersion = packet.Read<std::uint16_t>();
		if (procotolVersion != dcclite::PROTOCOL_VERSION)
		{
			dcclite::Log::Error("[DccLiteService::{}] [OnNet_Hello] Hello from {} - {} with invalid protocol version {}, expected {}, ignoring",
				this->GetName(),
				name,
				senderAddress,
				procotolVersion,
				dcclite::PROTOCOL_VERSION
			);

			return;
		}

		dcclite::Log::Info("[DccLiteService::{}] [OnNet_Hello] received hello from {}, starting handshake", this->GetName(), name);

		EventHub::PostEvent<NetworkHelloEvent>(std::ref(*this), senderAddress, name, remoteSessionToken, remoteConfigToken);
	}

	void DccLiteService::OnNetEvent_Hello(const dcclite::NetworkAddress &senderAddress, const std::string &deviceName, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken)
	{		
		//lookup device
		auto dev = this->TryFindDeviceByName(deviceName);
		NetworkDevice *netDevice;
		if (dev == nullptr)
		{
			//no device, create a temp one
			dcclite::Log::Warn("[DccLiteService::{}] [OnNetEvent_Hello] {} is not on config", this->GetName(), deviceName);

			netDevice = static_cast<NetworkDevice *>(m_pDevices->AddChild(
				std::make_unique<NetworkDevice>(
					deviceName,
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
				dcclite::Log::Error("[DccLiteService::{}] [OnNet_Hello] {} is not a network device, cannot accept connection. Please check config.", this->GetName(), deviceName);

				return;
			}
		}

		netDevice->AcceptConnection(dcclite::Clock::DefaultClock_t::now(), senderAddress, remoteSessionToken, remoteConfigToken);
	}

	class GenericNetworkEvent: public dcclite::broker::EventHub::IEvent
	{
		public:
			GenericNetworkEvent(DccLiteService &target, dcclite::NetworkAddress address, const dcclite::Packet &packet, dcclite::MsgTypes msgType):
				IEvent(target),				
				m_clAddress(address),
				m_clPacket{ packet },
				m_tMsgType{ msgType }
			{
				//empty
			}

			void Fire() override
			{
				static_cast<DccLiteService &>(this->GetTarget()).OnNetEvent_Packet(m_clAddress, m_clPacket, m_tMsgType);
			}

		private:
			const dcclite::NetworkAddress	m_clAddress;

			dcclite::Packet					m_clPacket;

			dcclite::MsgTypes				m_tMsgType;
	};

	void DccLiteService::OnNetEvent_Packet(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet, const dcclite::MsgTypes msgType)
	{
		auto dev = TryFindPacketDestination(packet);
		if (!dev)
		{
			dcclite::Log::Warn("[DccLiteService::{}] [OnNetEvent_Packet] Received packet from unkown device", this->GetName());

			return;
		}

		dcclite::Guid configToken = packet.ReadGuid();

		dev->OnPacket(packet, dcclite::Clock::DefaultClock_t::now(), msgType, senderAddress, configToken);
	}

	void DccLiteService::NetworkThreadProc()
	{
		std::uint8_t data[PACKET_MAX_SIZE];

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
				dcclite::Log::Error("[DccLiteService] [NetworkThreadProc::Update] packet size too big, truncating");

				size = dcclite::PACKET_MAX_SIZE;
			}

			dcclite::Packet pkt{ data, static_cast<uint8_t>(size) };

			//dcclite::PacketReader reader{ pkt };	

			if (pkt.Read<uint32_t>() != dcclite::PACKET_ID)
			{
				dcclite::Log::Warn("[DccLiteService] [NetworkThreadProc::Update] Invalid packet id");

				return;
			}

			auto msgType = static_cast<dcclite::MsgTypes>(pkt.Read<uint8_t>());

			switch (msgType)
			{
				case dcclite::MsgTypes::DISCOVERY:
					this->NetworkThread_OnDiscovery(sender, pkt);
					break;

				case dcclite::MsgTypes::HELLO:
					this->NetworkThread_OnNetHello(sender, pkt);
					break;

				default:
					dcclite::broker::EventHub::PostEvent<GenericNetworkEvent>(std::ref(*this), sender, pkt, msgType);
					break;
			}
		}
	}	
}

