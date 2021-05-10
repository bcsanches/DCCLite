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

#include "Service.h"

#include <map>
#include <string>
#include <vector>

#include "Decoder.h"
#include "Guid.h"
#include "IDccLiteService.h"
#include "Packet.h"

#include "Socket.h"

namespace dcclite::broker
{
	class Device;
	class NetworkDevice;
	class LocationManager;
	class OutputDecoder;
	class SimpleOutputDecoder;
	class SensorDecoder;
	class TurnoutDecoder;

	class DccLiteService : public Service, private IDccLite_DeviceServices, private IDccLite_DecoderServices
	{
		public:
			DccLiteService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			~DccLiteService() override;
		
			void Update(const dcclite::Clock &clock) override;				

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

			Decoder* TryFindDecoder(const DccAddress address) const;
			Decoder *TryFindDecoder(std::string_view id) const override;

			//This returns only pure outputs, turnouts are ignored
			std::vector<SimpleOutputDecoder *> FindAllSimpleOutputDecoders();

			std::vector<SensorDecoder*> FindAllSensorDecoders();

			std::vector<TurnoutDecoder*> FindAllTurnoutDecoders();

		private:
			void OnNet_Discovery(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);
			void OnNet_Hello(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet);		

			void OnNet_Packet(const dcclite::Clock &clock, const dcclite::NetworkAddress &senderAddress, dcclite::Packet &packet, const dcclite::MsgTypes msgType);

			Device *TryFindDeviceByName(std::string_view name);

			NetworkDevice *TryFindDeviceSession(const dcclite::Guid &guid);

			NetworkDevice *TryFindPacketDestination(dcclite::Packet &packet);							

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

			void Device_NotifyInternalItemCreated(const dcclite::IObject &item) const override;
			void Device_NotifyInternalItemDestroyed(const dcclite::IObject &item) const override;

			//
			//
			// To be used only by Decoders
			//
			//

			void Decoder_OnStateChanged(Decoder& decoder) override;

		private:
			dcclite::Socket m_clSocket;		

			FolderObject *m_pDecoders;
			FolderObject *m_pAddresses;
			FolderObject *m_pDevices;
			FolderObject *m_pSessions;

			LocationManager *m_pLocations;
	};
}

