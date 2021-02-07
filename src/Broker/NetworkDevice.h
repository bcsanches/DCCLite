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
#include <variant>

#include "IDccLiteService.h"
#include "Device.h"
#include "IDevice.h"
#include "Packet.h"
#include "PinManager.h"
#include "Socket.h"

#include <rapidjson/document.h>

class NetworkDevice: public Device, public INetworkDevice_DecoderServices
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE			
		};		

	public:
		NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project);
		NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const Project &project);

		NetworkDevice(const NetworkDevice &) = delete;
		NetworkDevice(NetworkDevice &&) = delete;

		~NetworkDevice();

		void Update(const dcclite::Clock &clock) override;

		void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::NetworkAddress remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

		void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time, const dcclite::MsgTypes msgType, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteConfigToken);		
		
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

		//
		//IDevice_DecoderServices
		//
		//
		
		INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept override
		{
			return this;
		}

		//
		//INetworkDevice_DecoderServices
		//
		//

		void Decoder_RegisterPin(const Decoder &decoder, dcclite::BasicPin pin, const char *usage)
		{
			m_clPinManager.RegisterPin(decoder, pin, usage);
		}

		void Decoder_UnregisterPin(const Decoder &decoder, dcclite::BasicPin pin)
		{
			m_clPinManager.UnregisterPin(decoder, pin);
		}	

	protected:
		void OnUnload() override;

	private:
		bool CheckSessionConfig(const dcclite::Guid remoteConfigToken, const dcclite::NetworkAddress remoteAddress);
		bool CheckSession(const dcclite::NetworkAddress remoteAddress);

		void GoOffline();
		void Disconnect();

		void RefreshTimeout(const dcclite::Clock::TimePoint_t time);
		bool CheckTimeout(const dcclite::Clock::TimePoint_t time);				
		
		void ClearState();
		void GotoSyncState();
		void GotoOnlineState();
		void GotoConfigState(const dcclite::Clock::TimePoint_t time);						

	private:						
		PinManager				m_clPinManager;		

		//
		//
		//Remote Device Info				
		dcclite::Guid		m_SessionToken;

		dcclite::NetworkAddress	m_RemoteAddress;

		//
		//
		//Connection status
		Status				m_eStatus;

		dcclite::Clock::TimePoint_t m_Timeout;					

		struct State
		{
			virtual void OnPacket(
				NetworkDevice &self,
				dcclite::Packet &packet, 
				const dcclite::Clock::TimePoint_t time, 
				const dcclite::MsgTypes msgType, 
				const dcclite::NetworkAddress remoteAddress, 
				const dcclite::Guid remoteConfigToken
			);

			virtual void Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time) = 0;
			virtual const char *GetName() const = 0;				
		};		

		struct ConfigState: State
		{
			std::vector<bool>	m_vecAcks;

			dcclite::Clock::TimePoint_t m_RetryTime;

			uint8_t				m_uSeqCount = { 0 };
			bool				m_fAckReceived = { false };

			ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time);			

			void OnPacket(
				NetworkDevice &self,
				dcclite::Packet &packet,
				const dcclite::Clock::TimePoint_t time,
				const dcclite::MsgTypes msgType,
				const dcclite::NetworkAddress remoteAddress,
				const dcclite::Guid remoteConfigToken
			) override;

			void Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time) override;

			const char *GetName() const override { return "ConfigState"; }

			private:				
				void SendDecoderConfigPacket(const NetworkDevice &self, const size_t index) const;
				void SendConfigStartPacket(const NetworkDevice &self) const;
				void SendConfigFinishedPacket(const NetworkDevice &self) const;

				void OnPacket_ConfigAck(
					NetworkDevice &self,
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				);

				void OnPacket_ConfigFinished(
					NetworkDevice &self,
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				);
		};			

		struct SyncState: State
		{
			dcclite::Clock::TimePoint_t m_SyncTimeout;

			void OnPacket(
				NetworkDevice &self,
				dcclite::Packet &packet,
				const dcclite::Clock::TimePoint_t time,
				const dcclite::MsgTypes msgType,
				const dcclite::NetworkAddress remoteAddress,
				const dcclite::Guid remoteConfigToken
			) override;

			void Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time) override;

			const char *GetName() const override { return "SyncState"; }
		};

		struct OnlineState: State
		{			
			OnlineState();

			void OnPacket(
				NetworkDevice &self,
				dcclite::Packet &packet,
				const dcclite::Clock::TimePoint_t time,
				const dcclite::MsgTypes msgType,
				const dcclite::NetworkAddress remoteAddress,
				const dcclite::Guid remoteConfigToken
			) override;

			void Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time) override;

			const char *GetName() const override { return "OnlineState"; }

			private:
				void SendStateDelta(NetworkDevice &self, const bool sendSensorsState, const dcclite::Clock::TimePoint_t time);

				dcclite::Clock::TimePoint_t m_tLastStateSentTime;
				dcclite::StatesBitPack_t	m_tLastStateSent;

				uint64_t			m_uLastReceivedStatePacketId = 0;

				uint64_t			m_uOutgoingStatePacketId = 0;
		};


		//
		//
		//Connection state
		struct NullState {};
		
		std::variant< NullState, ConfigState, SyncState, OnlineState> m_vState;
		State *m_pclCurrentState = nullptr;

		/**
		Registered is a device that is stored on config.

		Devices that contact the Broker, but are not in the config files, are marked as unregistered

		*/				
		bool					m_fRegistered;		
};
