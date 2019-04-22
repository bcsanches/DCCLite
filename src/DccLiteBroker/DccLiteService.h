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

#include "Decoder.h"
#include "Guid.h"
#include "Packet.h"

#include "Socket.h"

class Device;

class DccLiteService : public Service
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, const rapidjson::Value &params, const Project &project);

		virtual ~DccLiteService();

		Decoder &Create(
			const std::string &className,
			Decoder::Address address,
			const std::string &name,
			const rapidjson::Value &params
		);

		virtual void Update(const dcclite::Clock &clock) override;		

		//
		// To be used only by Devices
		//
		//
		void Device_PreparePacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, const dcclite::Guid &sessionToken, const dcclite::Guid &configToken);
		void Device_SendPacket(const dcclite::Address destination, const dcclite::Packet &packet);

		void Device_RegisterSession(Device &dev, const dcclite::Guid &configToken);
		void Device_UnregisterSession(const dcclite::Guid &sessionToken);

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "DccLiteService";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Service::Serialize(stream);

			//nothing
		}

		Decoder *TryFindDecoder(std::string_view id);

	private:
		void OnNet_Hello(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_Ping(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_ConfigAck(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_ConfigFinished(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_State(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);

		Device *TryFindDeviceByName(std::string_view name);

		Device *TryFindDeviceSession(const dcclite::Guid &guid);

		Device *DccLiteService::TryFindPacketDestination(dcclite::Packet &packet);		

	private:
		dcclite::Socket m_clSocket;		

		FolderObject *m_pDecoders;
		FolderObject *m_pAddresses;
		FolderObject *m_pDevices;
		FolderObject *m_pSessions;
};


