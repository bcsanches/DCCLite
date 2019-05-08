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

#include <string>
#include <vector>

#include "Clock.h"
#include "Guid.h"
#include "Object.h"
#include "Packet.h"
#include "Socket.h"

#include <rapidjson/document.h>

class DccLiteService;
class Decoder;
class Project;

class Device: public dcclite::FolderObject
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE			
		};		

	public:
		Device(std::string name, DccLiteService &dccService, const rapidjson::Value &params, const Project &project);
		Device(std::string name, DccLiteService &dccService);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

		void Update(const dcclite::Clock &clock);

		void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

		void OnPacket_ConfigAck(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress);
		void OnPacket_ConfigFinished(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress);
		void OnPacket_Ping(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken);
		void OnPacket_State(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken);
		
		inline const dcclite::Guid &GetConfigToken() noexcept
		{
			return m_ConfigToken;
		}

		inline bool IsOnline() const noexcept
		{
			return m_eStatus == Status::ONLINE;
		}

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "Device";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const;

	private:
		bool CheckSessionConfig(dcclite::Guid remoteConfigToken, dcclite::Address remoteAddress);
		bool CheckSession(dcclite::Address remoteAddress);

		void GoOnline(const dcclite::Address remoteAddress);
		void GoOffline();

		void RefreshTimeout(dcclite::Clock::TimePoint_t time);
		bool CheckTimeout(dcclite::Clock::TimePoint_t time);

		void SendDecoderConfigPacket(size_t index) const;
		void SendConfigStartPacket() const;
		void SendConfigFinishedPacket() const;

		void ForceSync();
		

	private:		
		DccLiteService			&m_clDccService;					

		std::vector<Decoder *>	m_vecDecoders;

		//
		//
		//Remote Device Info		
		dcclite::Guid		m_ConfigToken;
		dcclite::Guid		m_SessionToken;

		dcclite::Address	m_RemoteAddress;

		Status				m_eStatus;

		dcclite::Clock::TimePoint_t m_Timeout;

		uint64_t			m_uLastReceivedStatePacketId = 0;
		uint64_t			m_uOutgoingStatePacketId = 0;		

		struct ConfigInfo
		{
			std::vector<bool>	m_vecAcks;

			dcclite::Clock::TimePoint_t m_RetryTime;

			uint8_t				m_uSeqCount = { 0 };			
			bool				m_fAckReceived = { false };
		};		

		std::unique_ptr<ConfigInfo> m_upConfigState;

		/**
		Registered is a device that is stored on config.

		Devices that contact the Brooker, but are not in the config files, are marked as unregistered

		*/
		bool					m_fRegistered;
};
