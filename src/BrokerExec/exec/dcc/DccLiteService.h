// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <map>
#include <string>
#include <thread>
#include <vector>

#include <dcclite_shared/GuidDefs.h>
#include <dcclite_shared/Packet.h>

#include <dcclite/Socket.h>

#include "Decoder.h"
#include "IDccLiteService.h"

#include "sys/Service.h"
#include "sys/EventHub.h"
#include "sys/Thinker.h"

namespace dcclite::broker::exec::dcc
{
	class Device;
	class NetworkDevice;
	class LocationManager;
	class OutputDecoder;
	class SimpleOutputDecoder;	
	class StateDecoder;
	class TurnoutDecoder;	

	class DccLiteService: public sys::Service, private IDccLite_DeviceServices, private IDccLite_DecoderServices, public sys::EventHub::IEventTarget
	{
		public:
			static const char *TYPE_NAME;

			DccLiteService(RName name, sys::Broker &broker, const rapidjson::Value &params);

			~DccLiteService() override;			

			static void RegisterFactory();

			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override
			{
				Service::Serialize(stream);

				//nothing
			}

			void ClearBlockList();

			//
			//DECODERS Management
			//
			//

			Decoder *TryFindDecoder(const Address address) const;
			Decoder *TryFindDecoder(RName name) const override;

			//
			//
			// DEVICES management
			//
			//			
			Device *TryFindDeviceByName(RName name);

			//
			// 
			// Queries, used most by the JMRI x DccLite bridge (DccppService) 
			// 
			//

			//This returns only pure outputs, turnouts are ignored
			std::vector<const SimpleOutputDecoder *> FindAllSimpleOutputDecoders() const;

			std::vector<const StateDecoder *> FindAllInputDecoders() const;

			std::vector<const TurnoutDecoder *> FindAllTurnoutDecoders() const;			

		private:			
			void OnNetEvent_Hello(
				const dcclite::NetworkAddress	&senderAddress, 
				RName							name, 
				const dcclite::Guid				remoteSessionToken, 
				const dcclite::Guid				remoteConfigToken,
				const std::uint16_t				protocolVersion
			);

			template <typename T>
			std::vector<const T *> FindAllDecoders() const;

			void OnNetEvent_Packet(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet, const dcclite::MsgTypes msgType);

			NetworkDevice *TryFindDeviceSession(const dcclite::Guid &guid);

			NetworkDevice *TryFindPacketDestination(dcclite::Packet &packet);				

			void NetworkThreadProc();

			void NetworkThread_OnDiscovery(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);
			void NetworkThread_OnNetHello(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);

			friend class GenericNetworkEvent;
			friend class NetworkHelloEvent;

			//
			//
			// Block List Management
			//
			//			

			[[nodiscard]]
			bool IsDeviceBlocked(const dcclite::NetworkAddress &address) const noexcept;

		private:
			//
			// To be used only by Devices
			//
			//		
			void Device_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet& packet) override;

			void Device_RegisterSession(NetworkDevice &dev, const dcclite::Guid& configToken) override;
			void Device_UnregisterSession(NetworkDevice &dev, const dcclite::Guid& sessionToken) override;

			void Device_DestroyUnregistered(NetworkDevice &dev) override;

			friend class DestroyUnregisteredDeviceEvent;

			Decoder& Device_CreateDecoder(
				IDevice_DecoderServices &dev,
				std::string_view className,
				Address address,
				RName name,
				const rapidjson::Value &params
			) override;			

			void Device_DestroyDecoder(Decoder &dec) override;

			void Device_NotifyInternalItemCreated(dcclite::IObject &item) const override;
			void Device_NotifyInternalItemDestroyed(dcclite::IObject &item) const override;
			void Device_NotifyStateChange(NetworkDevice &device) const override;

			void Device_Block(NetworkDevice &dev) override;

			//
			//
			// To be used only by Decoders
			//
			//

			void Decoder_OnStateChanged(Decoder& decoder) override;		

		private:			
			dcclite::Socket m_clSocket;		

			std::thread		m_clNetworkThread;

			FolderObject *m_pDecoders;			
			FolderObject *m_pDevices;
			FolderObject *m_pSessions;
			FolderObject *m_pBlockedDevices = nullptr;

			/// <summary>
			/// Keep track of existing addresses. No pratical use, just to track and avoid duplicate addresses
			/// </summary>
			FolderObject *m_pAddresses;

			/// <summary>
			/// Keep a listing of decimal addresses for easier inspection on SharpTerminal
			/// </summary>
			FolderObject *m_pDecAddresses;

			LocationManager *m_pLocations;
	};
}

