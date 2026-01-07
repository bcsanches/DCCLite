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

#include <exception>

#include <dcclite/FmtUtils.h>
#include <dcclite/Guid.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>
#include <dcclite/Util.h>

#include "magic_enum/magic_enum.hpp"

#include "NetworkDevice.h"
#include "LocationManager.h"
#include "QuadInverter.h"
#include "SensorDecoder.h"
#include "SignalDecoder.h"
#include "SimpleOutputDecoder.h"
#include "TurnoutDecoder.h"
#include "TurntableAutoInverterDecoder.h"
#include "VirtualDevice.h"
#include "VirtualTurnoutDecoder.h"
#include "VirtualSensorDecoder.h"

#include "sys/EventHub.h"
#include "sys/ServiceFactory.h"

using namespace std::chrono_literals;

namespace dcclite::broker::exec::dcc
{	
	static std::unique_ptr<Decoder> TryCreateDecoder(std::string_view className, Address address, RName name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params)
	{
		//
		//Check the most common first....
		//
		if (className.compare("ServoTurnout") == 0)
			return std::make_unique<ServoTurnoutDecoder>(address, name, owner, dev, params);

		if (className.compare("Output") == 0)
			return std::make_unique<SimpleOutputDecoder>(address, name, owner, dev, params);

		if (className.compare("Sensor") == 0)
			return std::make_unique<SensorDecoder>(address, name, owner, dev, params);

		if (className.compare("VirtualSignal") == 0)
			return std::make_unique<SignalDecoder>(address, name, owner, dev, params);

		if (className.compare("VirtualTurnout") == 0)
			return std::make_unique<VirtualTurnoutDecoder>(address, name, owner, dev, params);
		
		if (className.compare("QuadInverter") == 0)
			return std::make_unique<QuadInverter>(address, name, owner, dev, params);
		
		if (className.compare("TurntableAutoInverter") == 0)
			return std::make_unique<TurntableAutoInverterDecoder>(address, name, owner, dev, params);

		if (className.compare(VIRTUAL_SENSOR_DECODER_CLASSNAME) == 0)
			return std::make_unique<VirtualSensorDecoder>(address, name, owner, dev, params);
					
		return nullptr;
	}

	//
	//
	//
	//
	//

	const char *DccLiteService::TYPE_NAME = "DccLiteService";	
	static sys::GenericServiceFactory<DccLiteService> g_DccLiteServiceFactory;

	void DccLiteService::RegisterFactory()
	{
		//empty
	}


	DccLiteService::DccLiteService(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		Service(name, broker, params)		
	{
		BenchmarkLogger benchmark{ "DccLiteService", name.GetData() };

		m_pDecoders = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "decoders" })));
		m_pAddresses = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "addresses" })));
		m_pDecAddresses = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "dec_addresses" })));
		m_pDevices = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "devices" })));
		m_pSessions = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "sessions" })));

		m_pLocations = static_cast<LocationManager *>(this->AddChild(std::make_unique<LocationManager>(RName{ "locations" }, params)));

		uint16_t port = json::TryGetDefaultInt(params, "port", dcclite::DEFAULT_DCCLITE_SERVER_PORT);

		[[unlikely]]
		if (!m_clSocket.Open(port, dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error(fmt::format("[DccLiteService::{}] error: cannot open socket", name));
		}

		dcclite::Log::Info("[DccLiteService::{}] Listening on port {}", this->GetName(), port);

		const auto devicesArray = json::GetArray(params, "devices", "DccLiteService");
		
		try
		{
			for (auto &device : devicesArray)
			{
				RName nodeName{ json::GetString(device, "name", "device data for DccLiteService")};
				auto className = json::GetString(device, "class", "device data for DccLiteService");

				if (className.compare("Virtual"))
					m_pDevices->AddChild(std::make_unique<NetworkDevice>(nodeName, broker, *static_cast<IDccLite_NetworkDeviceServices *>(this), device));
				else
					m_pDevices->AddChild(std::make_unique<VirtualDevice>(nodeName, broker, *static_cast<IDccLite_DeviceServices *>(this), device));
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
		dcclite::Log::Info("[DccLiteService::{}] cleanup", this->GetName());

		//Destroy devices, so they clean everything
		this->RemoveChild(m_pDevices->GetName());

		dcclite::Log::Info("[DccLiteService::{}] Closing socket", this->GetName());

		m_clSocket.Close();

		dcclite::Log::Info("[DccLiteService::{}] Joining network thread", this->GetName());

		m_clNetworkThread.join();		

		dcclite::Log::Info("[DccLiteService::{}] destructor done", this->GetName());
	}

	void DccLiteService::Device_DestroyDecoder(Decoder &dec)
	{
		//send mesage before we destroy it
		this->NotifyItemDestroyed(dec);

		const auto &address = dec.GetAddress();

		auto addressName = RName{ address.ToString() };
		this->NotifyItemDestroyed(*(m_pAddresses->TryGetChild(addressName)));		

		m_pLocations->UnregisterDecoder(dec);

		m_pAddresses->RemoveChild(addressName);
		m_pDecoders->RemoveChild(dec.GetName());	
		m_pDecAddresses->RemoveChild(RName{ address.ToDecimalString() });
	}

	Decoder &DccLiteService::Device_CreateDecoder(
		IDevice_DecoderServices &dev,
		std::string_view className,
		Address address,
		RName name,
		const rapidjson::Value &params
	)
	{
		auto decoder = TryCreateDecoder(className, address, name, *this, dev, params);	

		[[unlikely]]
		if (!decoder)
		{				
			throw std::runtime_error(fmt::format("[DccLiteService::{}] [Device_CreateDecoder] Error: failed to instantiate decoder {} - {}", name, address, className));
		}

		auto pDecoder = decoder.get();	

		m_pDecoders->AddChild(std::move(decoder));		

		RName decimalAddress{ address.ToDecimalString() };
		RName hexAddress{ address.ToString() };

		try
		{			
			auto decimalAddressShortcut = m_pDecAddresses->AddChild(
				std::make_unique<dcclite::Shortcut>(
					decimalAddress,
					*pDecoder
				)
			);

			auto addressShortcut = m_pAddresses->AddChild(
				std::make_unique<dcclite::Shortcut>(
					hexAddress,
					*pDecoder
				)
			);

			this->NotifyItemCreated(*pDecoder);
			this->NotifyItemCreated(*addressShortcut);
			this->NotifyItemCreated(*decimalAddressShortcut);
		}
		catch (...)
		{	
			//this if should never fail...
			[[likely]]
			if (auto existingDec = dynamic_cast<Shortcut *>(m_pAddresses->TryGetChild(hexAddress)))
			{
				dcclite::Log::Warn("[DccLiteService::Device_CreateDecoder] Cannot create {}, decoder at address {} ({}) - {} already exists",
					name,
					decimalAddress,
					hexAddress,
					existingDec->TryResolve()->GetName()
				);
			}

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

	Device *DccLiteService::TryFindDeviceByName(RName name)
	{	
		return static_cast<Device *>(m_pDevices->TryGetChild(name));
	}

	NetworkDevice *DccLiteService::TryFindDeviceSession(const dcclite::Guid &guid)
	{
		return static_cast<NetworkDevice *>(m_pSessions->TryResolveChild(RName{ dcclite::GuidToString(guid) }));
	}	

	NetworkDevice *DccLiteService::TryFindPacketDestination(dcclite::Packet &packet)
	{	
		dcclite::Guid sessionToken = packet.ReadGuid();	

		auto dev = this->TryFindDeviceSession(sessionToken);

		[[unlikely]]
		if (dev == nullptr)
		{
			dcclite::Log::Warn("[DccLiteService::{}] [TryFindPacketDestination] Received packet from unknown session", this->GetName());

			return nullptr;
		}

		return dev;
	}

	void DccLiteService::NetworkDevice_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet &packet)
	{
		[[unlikely]]
		if (!m_clSocket.Send(destination, packet.GetData(), packet.GetSize()))
		{
			dcclite::Log::Error("[DccLiteService::{}] [NetworkDevice_SendPacket] Failed to send packet to {}", this->GetName(), destination);
		}
	}

	void DccLiteService::NetworkDevice_RegisterSession(NetworkDevice &dev, const dcclite::Guid &sessionToken)
	{
		auto session = m_pSessions->AddChild(std::make_unique<dcclite::Shortcut>(RName{ dcclite::GuidToString(sessionToken) }, dev));

		this->NotifyItemCreated(*session);
	}

	void DccLiteService::NetworkDevice_UnregisterSession(NetworkDevice& dev, const dcclite::Guid &sessionToken)
	{	
		auto session = m_pSessions->RemoveChild(RName{ dcclite::GuidToString(sessionToken) });
					
		this->NotifyItemDestroyed(*session);
	}

	void DccLiteService::NetworkDevice_NotifyStateChange(NetworkDevice &device, dcclite::broker::sys::ObjectManagerEvent::SerializeDeltaProc_t proc) const
	{
		this->NotifyItemChanged(device, proc);
	}

	class DestroyUnregisteredDeviceEvent: public dcclite::broker::sys::EventHub::IEvent
	{
		public:
			DestroyUnregisteredDeviceEvent(DccLiteService &target, RName deviceName):
				IEvent(target),
				m_rnDeviceName{ deviceName }
			{
				//empty
			}

			void Fire() override
			{
				auto &service = static_cast<DccLiteService &>(this->GetTarget());

				dcclite::Log::Info("[DccLiteService::DestroyUnregisteredDeviceEvent] Destroying {} device", m_rnDeviceName);

				NetworkDevice *device = static_cast<NetworkDevice *>(service.m_pDevices->TryGetChild(m_rnDeviceName));
				if (!device)
				{
					dcclite::Log::Info("[DccLiteService::DestroyUnregisteredDeviceEvent] Device {} not found!", m_rnDeviceName);

					return;
				}

				if (device->IsConnectionStable())
				{
					dcclite::Log::Info("[DccLiteService::DestroyUnregisteredDeviceEvent] Device {} is connected, aborting!", m_rnDeviceName);

					return;
				}

				auto item = service.m_pDevices->RemoveChild(m_rnDeviceName);
				if (!item)
				{
					dcclite::Log::Warn("[DccLiteService::DestroyUnregisteredDeviceEvent] Device {} not found", m_rnDeviceName);

					return;
				}

				service.NotifyItemDestroyed(*item.get());
			}

		private:			
			RName m_rnDeviceName;
	};

	void DccLiteService::NetworkDevice_DestroyUnregistered(NetworkDevice &dev)
	{
		const auto name = dev.GetName();

		if (dev.IsConnectionStable())
			throw std::runtime_error(fmt::format("[DccLiteService::Device_DestroyUnregistered] Cannot destroy connected device {}", name));

		dcclite::Log::Info("[DccLiteService::Device_DestroyUnregistered] Requested to destroy {} device", name);

		//fire a event to later destroy the device... to avoid functions touching a dead device under the callstack
		sys::EventHub::PostEvent<DestroyUnregisteredDeviceEvent>(std::ref(*this), name);
	}

	Decoder* DccLiteService::TryFindDecoder(const Address address) const
	{
		//Name is registered?
		auto rname = RName::TryGetName(address.ToString());
		if (!rname)
			return nullptr;

		return static_cast<Decoder *>(m_pAddresses->TryResolveChild(rname));
	}

	Decoder *DccLiteService::TryFindDecoder(RName id) const
	{
		auto *decoder = m_pAddresses->TryResolveChild(id);

		return static_cast<Decoder *>(decoder ? decoder : m_pDecoders->TryResolveChild(id));
	}

	template <typename T>
	std::vector<const T *> DccLiteService::FindAllDecoders() const
	{
		std::vector<const T *> vecDecoders;

		m_pDecoders->VisitChildren([&vecDecoders](auto &obj)
			{
				if (auto decoder = dynamic_cast<const T *>(&obj))
					vecDecoders.push_back(decoder);

				return true;
			}
		);

		return vecDecoders;
	}

	std::vector<const SimpleOutputDecoder *> DccLiteService::FindAllSimpleOutputDecoders() const
	{
		return this->FindAllDecoders<SimpleOutputDecoder>();
	}

	std::vector<const StateDecoder *> DccLiteService::FindAllInputDecoders() const
	{
		return this->FindAllDecoders<StateDecoder>();		
	}

	std::vector<const TurnoutDecoder *> DccLiteService::FindAllTurnoutDecoders() const
	{
		return this->FindAllDecoders<TurnoutDecoder>();		
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

	void DccLiteService::NetworkThread_OnDiscovery(const dcclite::NetworkAddress &senderAddress, const dcclite::Packet &packet)
	{
		dcclite::Log::Info("[DccLiteService::{}] [OnNet_Hello] received discovery from {}, sending reply", this->GetName(), senderAddress);

		dcclite::Packet pkt;

		//just send a blank packet, so they know we are here
		dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::DISCOVERY, dcclite::Guid{}, dcclite::Guid{} };

		this->NetworkDevice_SendPacket(senderAddress, pkt);
	}

	class NetworkHelloEvent: public sys::EventHub::IEvent
	{
		public:
			NetworkHelloEvent(DccLiteService &target, dcclite::NetworkAddress address, RName deviceName, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken, uint16_t protocolVersion):
				IEvent(target),
				m_clAddress(address),
				m_clRemoteSessionToken{ remoteSessionToken },
				m_clRemoteConfigToken{ remoteConfigToken },
				m_uProtocolVersion{ protocolVersion },
				m_rnDeviceName{ deviceName }
			{
				//empty
			}

			void Fire() override
			{
				static_cast<DccLiteService &>(this->GetTarget()).OnNetEvent_Hello(m_clAddress, m_rnDeviceName, m_clRemoteSessionToken, m_clRemoteConfigToken, m_uProtocolVersion);
			}

		private:
			const dcclite::NetworkAddress m_clAddress;

			const dcclite::Guid m_clRemoteSessionToken;
			const dcclite::Guid m_clRemoteConfigToken;

			const uint16_t		m_uProtocolVersion;

			RName m_rnDeviceName;
	};

	static std::string MakeIpName(const dcclite::NetworkAddress& address)
	{
		return fmt::format("IP_{}_{}", address, address.GetPort());
	}

	void DccLiteService::NetworkThread_OnNetHello(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet)
	{
		auto remoteSessionToken = packet.ReadGuid();
		auto remoteConfigToken = packet.ReadGuid();

		dcclite::PacketReader reader(packet);

		char name[256];
		reader.ReadStr(name, sizeof(name));

		const auto procotolVersion = packet.Read<std::uint16_t>();

		[[unlikely]]
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

		if (!name[0])
		{
			dcclite::Log::Warn("[DccLiteService::{}] [OnNet_Hello] Hello from {} with blank name.",
				this->GetName(),				
				senderAddress
			);

			auto newName = MakeIpName(senderAddress);
			strcpy(name, newName.c_str());
		}

		dcclite::Log::Info("[DccLiteService::{}] [OnNet_Hello] received hello from {}, starting handshake", this->GetName(), name);

		sys::EventHub::PostEvent<NetworkHelloEvent>(std::ref(*this), senderAddress, RName{ name }, remoteSessionToken, remoteConfigToken, procotolVersion);
	}

	void DccLiteService::OnNetEvent_Hello(
		const dcclite::NetworkAddress &senderAddress, 
		RName deviceName, 
		const dcclite::Guid remoteSessionToken, 
		const dcclite::Guid remoteConfigToken,
		const std::uint16_t protocolVersion
	)
	{
		if (this->IsDeviceBlocked(senderAddress))
		{
			dcclite::Log::Warn("[DccLiteService::{}] [OnNetEvent_Hello] {} - {} is blocked, ignoring hello request", this->GetName(), deviceName, senderAddress);

			return;
		}

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
					*static_cast<IDccLite_NetworkDeviceServices *>(this)					
				)
			));

			this->NotifyItemCreated(*netDevice);
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

		netDevice->AcceptConnection(dcclite::Clock::DefaultClock_t::now(), senderAddress, remoteSessionToken, remoteConfigToken, protocolVersion);
	}

	class GenericNetworkEvent: public sys::EventHub::IEvent
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
			
			[[unlikely]]
			if (status != dcclite::Socket::Status::OK)
			{
				dcclite::Log::Warn("[DccLiteService] [NetworkThreadProc::Update] socket receive returned: {}", magic_enum::enum_name(status));

				//ignore connreset.. happens only with emulator so far and makes no sense with UDP
				if (status == dcclite::Socket::Status::CONNRESET)
					continue;

				break;
			}			

			[[unlikely]]
			if (size > dcclite::PACKET_MAX_SIZE)
			{
				dcclite::Log::Error("[DccLiteService] [NetworkThreadProc::Update] packet size too big, truncating");

				size = dcclite::PACKET_MAX_SIZE;
			}

			dcclite::Packet pkt{ data, static_cast<uint8_t>(size) };			

			[[unlikely]]
			if (pkt.Read<uint32_t>() != dcclite::PACKET_ID)
			{
				dcclite::Log::Warn("[DccLiteService] [NetworkThreadProc::Update] Invalid packet id");

				continue;
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

				[[likely]]
				default:
					sys::EventHub::PostEvent<GenericNetworkEvent>(std::ref(*this), sender, pkt, msgType);
					break;
			}
		}
	}	

	//
	//
	// Block List Management
	//
	//

	void DccLiteService::NetworkDevice_Block(NetworkDevice &dev)
	{
		if(!dev.IsConnectionStable())
			throw std::runtime_error(fmt::format("[{}::BlockDevice] Device {} is not connected", this->GetName(), dev.GetName()));

		//created blocked devices list on demand...
		bool create = m_pBlockedDevices == nullptr;
		if (create)
		{
			m_pBlockedDevices = static_cast<FolderObject *>(this->AddChild(std::make_unique<dcclite::FolderObject>(RName{ "blockedDevices" })));			
		}

		try
		{
			auto block = m_pBlockedDevices->AddChild(std::make_unique<Object>(RName{ MakeIpName(dev.GetRemoteAddress()) }));

			//Was list created now? Notify it...
			if (create)
			{				
				this->NotifyItemCreated(*m_pBlockedDevices);
			}

			//Notify new item
			this->NotifyItemCreated(*block);

			//Drop connection
			dev.DisconnectDevice();
		}		
		catch (...)
		{
			if (create)
			{
				//cleanup
				this->RemoveChild(m_pBlockedDevices->GetName());
				m_pBlockedDevices = nullptr;
			}
		}
	}	

	[[nodiscard]]
	bool DccLiteService::IsDeviceBlocked(const dcclite::NetworkAddress& address) const noexcept
	{
		if (!m_pBlockedDevices)
			return false;

		return m_pBlockedDevices->TryGetChild(RName{ MakeIpName(address) });
	}

	void DccLiteService::ClearBlockList()
	{
		if (!m_pBlockedDevices)
			return;

		m_pBlockedDevices->VisitChildren([this](IObject &item)
			{
				this->NotifyItemDestroyed(item);

				return true;
			}
		);

		this->NotifyItemDestroyed(*m_pBlockedDevices);
		this->RemoveChild(m_pBlockedDevices->GetName());

		m_pBlockedDevices = nullptr;
	}
}

