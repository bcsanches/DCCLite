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
#include "Packet.h"

#include "Socket.h"

class Device;
class OutputDecoder;
class SensorDecoder;
class TurnoutDecoder;

class IDccLiteServiceListener
{
	public:
		virtual void OnDeviceConnected(Device& device) = 0;
		virtual void OnDeviceDisconnected(Device& device) = 0;

		virtual void OnDecoderStateChange(Decoder& decoder) = 0;

		virtual ~IDccLiteServiceListener() 
		{
			//empty
		}
};

class IDccDecoderServices
{
	public:
		virtual void Decoder_OnStateChanged(Decoder& decoder) = 0;
};

class IDccDeviceServices
{
	public:
		virtual Decoder& Device_CreateDecoder(
			const std::string& className,
			Decoder::Address address,
			const std::string& name,
			const rapidjson::Value& params
		) = 0;
		
		virtual void Device_SendPacket(const dcclite::Address destination, const dcclite::Packet& packet) = 0;

		virtual void Device_RegisterSession(Device& dev, const dcclite::Guid& configToken) = 0;
		virtual void Device_UnregisterSession(Device& dev, const dcclite::Guid& sessionToken) = 0;
};

class DccLiteService : public Service, private IDccDeviceServices, private IDccDecoderServices
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

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

		void AddListener(IDccLiteServiceListener &listener);
		void RemoveListener(IDccLiteServiceListener &listener);

		//
		//DECODERS Management
		//
		//

		Decoder* TryFindDecoder(const Decoder::Address address) const;
		Decoder *TryFindDecoder(std::string_view id) const;

		//This returns only pure outputs, turnouts are ignored
		std::vector<OutputDecoder*> FindAllOutputDecoders();

		std::vector<SensorDecoder*> FindAllSensorDecoders();

		std::vector<TurnoutDecoder*> FindAllTurnoutDecoders();

	private:
		void OnNet_Discovery(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_Hello(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_Ping(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_ConfigAck(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_ConfigFinished(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_State(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);

		Device *TryFindDeviceByName(std::string_view name);

		Device *TryFindDeviceSession(const dcclite::Guid &guid);

		Device *TryFindPacketDestination(dcclite::Packet &packet);		

	private:
		//
		// To be used only by Devices
		//
		//		
		void Device_SendPacket(const dcclite::Address destination, const dcclite::Packet& packet) override;

		void Device_RegisterSession(Device& dev, const dcclite::Guid& configToken) override;
		void Device_UnregisterSession(Device& dev, const dcclite::Guid& sessionToken) override;

		Decoder& Device_CreateDecoder(
			const std::string& className,
			Decoder::Address address,
			const std::string& name,
			const rapidjson::Value& params
		) override;

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

		std::vector<IDccLiteServiceListener *> m_vecListeners;
};


