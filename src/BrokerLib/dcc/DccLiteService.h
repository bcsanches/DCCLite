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

#include "../sys/Service.h"

#include <map>
#include <string>
#include <thread>
#include <vector>

#include "Decoder.h"
#include "Guid.h"
#include "IDccLiteService.h"
#include "Packet.h"
#include "Socket.h"

#include "../sys/EventHub.h"
#include "../sys/ScriptService.h"
#include "../sys/Thinker.h"

namespace dcclite::broker
{
	class Device;
	class NetworkDevice;
	class LocationManager;
	class OutputDecoder;
	class SimpleOutputDecoder;
	class SensorDecoder;
	class TurnoutDecoder;

	class DecoderWeakPointer
	{
		public:
			DecoderWeakPointer(dcclite::broker::Decoder &decoder, dcclite::broker::Service &dccLiteService);

			inline Decoder *TryGetDecoder() const noexcept
			{
				return m_pclDecoder;
			}

			inline DccAddress GetAddress() const noexcept
			{
				return m_clAddress;
			}

			sigslot::signal< dcclite::broker::Decoder &> m_sigDecoderCreated;
			sigslot::signal< dcclite::broker::Decoder &> m_sigDecoderDestroy;

		private:
			void OnObjectManagerEvent(const dcclite::broker::ObjectManagerEvent &event);

		private:
			/**
			* Internal ref to the oficial decoder
			*
			* Note that if user changes the device config file, it will get unloaded and reloaded, this will invalidate the pointer and if the user removes the
			* decoder or the file fails to reload, it may stay null for a long time, so watch for a null pointer here....
			*
			*/
			dcclite::broker::Decoder	*m_pclDecoder;
			dcclite::broker::DccAddress	m_clAddress;

			sigslot::scoped_connection	m_slotObjectManagerConnection;
	};

	class DccLiteService : public Service, private IDccLite_DeviceServices, private IDccLite_DecoderServices, public EventHub::IEventTarget, public ScriptService::IScriptSupport
	{
		public:
			DccLiteService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			~DccLiteService() override;				

			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "DccLiteService";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override
			{
				Service::Serialize(stream);

				//nothing
			}			

			//
			//DECODERS Management
			//
			//

			Decoder *TryFindDecoder(const DccAddress address) const;
			Decoder *TryFindDecoder(std::string_view id) const override;

			//
			//
			// DEVICES management
			//
			//			
			Device *TryFindDeviceByName(std::string_view name);

			//
			// 
			// Queries, used most by the JMRI x DccLite bridge (DccppService) 
			// 
			//

			//This returns only pure outputs, turnouts are ignored
			std::vector<SimpleOutputDecoder *> FindAllSimpleOutputDecoders();

			std::vector<SensorDecoder *> FindAllSensorDecoders();

			std::vector<TurnoutDecoder *> FindAllTurnoutDecoders();

			//
			//
			// Scripting
			//
			//

			void IScriptSupport_RegisterProxy(sol::table &table) override;

			void IScriptSupport_OnVMInit(sol::state &state) override;
			void IScriptSupport_OnVMFinalize(sol::state &state) override;

		private:			
			void OnNetEvent_Hello(const dcclite::NetworkAddress &senderAddress, const std::string &deviceName, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken);

			void OnNetEvent_Packet(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet, const dcclite::MsgTypes msgType);

			NetworkDevice *TryFindDeviceSession(const dcclite::Guid &guid);

			NetworkDevice *TryFindPacketDestination(dcclite::Packet &packet);				

			void NetworkThreadProc();

			void NetworkThread_OnDiscovery(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);
			void NetworkThread_OnNetHello(const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);

			friend class GenericNetworkEvent;
			friend class NetworkHelloEvent;

		private:
			//
			// To be used only by Devices
			//
			//		
			void Device_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet& packet) override;

			void Device_RegisterSession(NetworkDevice& dev, const dcclite::Guid& configToken) override;
			void Device_UnregisterSession(NetworkDevice& dev, const dcclite::Guid& sessionToken) override;

			Decoder& Device_CreateDecoder(
				IDevice_DecoderServices &dev,
				const std::string& className,
				DccAddress address,
				const std::string& name,
				const rapidjson::Value& params
			) override;

			void Device_DestroyDecoder(Decoder &dec) override;

			void Device_NotifyInternalItemCreated(dcclite::IObject &item) const override;
			void Device_NotifyInternalItemDestroyed(dcclite::IObject &item) const override;
			void Device_NotifyStateChange(NetworkDevice &device) const override;

			//
			//
			// To be used only by Decoders
			//
			//

			void Decoder_OnStateChanged(Decoder& decoder) override;

			[[nodiscard]] std::string_view Decoder_GetSystemName() const noexcept override
			{
				return this->GetName();
			}

		private:			
			dcclite::Socket m_clSocket;		

			std::thread		m_clNetworkThread;

			FolderObject *m_pDecoders;
			FolderObject *m_pAddresses;
			FolderObject *m_pDevices;
			FolderObject *m_pSessions;

			LocationManager *m_pLocations;
	};
}

