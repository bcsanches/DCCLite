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

#include <functional>
#include <list>
#include <string>
#include <variant>

#include "IDccLiteService.h"
#include "Device.h"
#include "IDevice.h"
#include "NetworkDeviceTasks.h"
#include "Packet.h"
#include "PinManager.h"
#include "Socket.h"

#include "../sys/Thinker.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{ 	
	class ServoTurnoutDecoder;

	class NetworkDevice: public Device, private INetworkDevice_DecoderServices, private detail::INetworkDevice_TaskServices
	{
		public:
			enum class Status
			{
				OFFLINE,
				CONNECTING,
				ONLINE			
			};		

		public:
			NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project);
			NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const Project &project);

			NetworkDevice(const NetworkDevice &) = delete;
			NetworkDevice(NetworkDevice &&) = delete;

			~NetworkDevice();			

			void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::NetworkAddress remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time, const dcclite::MsgTypes msgType, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteConfigToken);		
					
			[[nodiscard]] inline const dcclite::Guid &GetConfigToken() noexcept
			{
				return m_ConfigToken;
			}				

			[[nodiscard]] uint8_t FindDecoderIndex(const Decoder &decoder) const override;

			[[nodiscard]] virtual Decoder &FindDecoder(const std::string_view name) const override;

			//
			//IObject
			//
			//			
			[[nodiscard]] const char *GetTypeName() const noexcept override
			{
				return "NetworkDevice";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			//
			//IDevice_DecoderServices
			//
			//					
			[[nodiscard]] INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept override
			{
				return this;
			}								

			//
			//
			//INetworkDevice_TaskServices
			//
			//

			void TaskServices_FillPacketHeader(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType) const noexcept override;

			void TaskServices_SendPacket(dcclite::Packet &packet) override;			

			bool IsConnectionStable() const noexcept override;

			void TaskServices_ForgetTask(NetworkTask &task) override;			

			//
			//
			// Tasks
			//
			//			
			[[nodiscard]] std::shared_ptr<NetworkTask> StartDownloadEEPromTask(NetworkTask::IObserver *observer, DownloadEEPromTaskResult_t &resultsStorage);
			[[nodiscard]] std::shared_ptr<NetworkTask> StartServoTurnoutProgrammerTask(NetworkTask::IObserver *observer, const std::string_view servoDecoderName);

		protected:
			void OnUnload() override;

			void CheckLoadedDecoder(Decoder &decoder) override;

		private:									
			[[nodiscard]] bool CheckSessionConfig(const dcclite::Guid remoteConfigToken, const dcclite::NetworkAddress remoteAddress);
			
			[[nodiscard]] bool CheckSession(const dcclite::NetworkAddress remoteAddress);

			/*
			* This methods sets the state as offline, such as in cases when a connection is dropped
			*
			*/
			void GoOffline();

			/*
			* Disconnect does a graceful disconnect telling the device that we are disconnecting, after that, it goes to OFFLINE state
			* 
			*/
			void Disconnect();			
		
			void ClearState();
			void GotoSyncState();
			void GotoOnlineState(const dcclite::Clock::TimePoint_t time);
			void GotoConfigState(const dcclite::Clock::TimePoint_t time);		

			void AbortPendingTasks();

			//
			//INetworkDevice_DecoderServices
			//
			//

			void Decoder_RegisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) override
			{
				m_clPinManager.RegisterPin(decoder, pin, usage);
			}

			void Decoder_UnregisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin) override
			{
				m_clPinManager.UnregisterPin(decoder, pin);
			}

			void Decoder_OnChangeStateRequest(const Decoder &decoder) noexcept override;

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
			Status				m_kStatus = Status::OFFLINE;			

			struct State
			{
				virtual void OnPacket(					
					dcclite::Packet &packet, 
					const dcclite::Clock::TimePoint_t time, 
					const dcclite::MsgTypes msgType, 
					const dcclite::NetworkAddress remoteAddress, 
					const dcclite::Guid remoteConfigToken
				);
								
				[[nodiscard]] virtual const char *GetName() const = 0;

				protected:
					State(NetworkDevice &self):
						m_rclSelf(self)
					{
						//empty
					}

					NetworkDevice &m_rclSelf;
			};		

			struct ConfigState: State
			{
				std::vector<bool>	m_vecAcks;				

				uint8_t				m_uSeqCount = { 0 };
				bool				m_fAckReceived = { false };

				Thinker				m_clTimeoutThinker;

				ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time);			

				void OnPacket(					
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				) override;				

				[[nodiscard]] 
				const char *GetName() const override { return "ConfigState"; }

				private:				
					void SendDecoderConfigPacket(const size_t index) const;
					void SendConfigStartPacket() const;
					void SendConfigFinishedPacket() const;

					void OnTimeout(const dcclite::Clock::TimePoint_t time);

					void OnPacket_ConfigAck(						
						dcclite::Packet &packet,
						const dcclite::Clock::TimePoint_t time,
						const dcclite::MsgTypes msgType,
						const dcclite::NetworkAddress remoteAddress,
						const dcclite::Guid remoteConfigToken
					);

					void OnPacket_ConfigFinished(
						dcclite::Packet &packet,
						const dcclite::Clock::TimePoint_t time,
						const dcclite::MsgTypes msgType,
						const dcclite::NetworkAddress remoteAddress,
						const dcclite::Guid remoteConfigToken
					);
			};			

			struct SyncState: State
			{				
				explicit SyncState(NetworkDevice &self);

				void OnPacket(
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				) override;				

				[[nodiscard]] const char *GetName() const override { return "SyncState"; }

				private:
					void OnTimeout(const dcclite::Clock::TimePoint_t time);

					Thinker				m_clTimeoutThinker;
			};

			struct OnlineState: State
			{			
				OnlineState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time);

				void OnPacket(
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				) override;				

				[[nodiscard]] const char *GetName() const override { return "OnlineState"; }

				void OnChangeStateRequest(const Decoder &decoder);

				private:
					bool SendStateDelta(const bool sendSensorsState, const dcclite::Clock::TimePoint_t time, const std::string_view requester);

					void OnPingThink(const dcclite::Clock::TimePoint_t time);
					void OnStateDeltaThink(const dcclite::Clock::TimePoint_t time);									

					uint64_t			m_uLastReceivedStatePacketId = 0;

					uint64_t			m_uOutgoingStatePacketId = 0;

					Thinker				m_clPingThinker;
					Thinker				m_clSendStateDeltaThinker;
			};

			class TimeoutController
			{
				public:
					explicit TimeoutController(NetworkDevice &owner);

					void Enable(const dcclite::Clock::TimePoint_t time);

					void Disable();

				private:
					void OnThink(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;

					NetworkDevice &m_rclOwner;
			};


			//
			//
			//Connection state
			struct NullState {};
		
			std::variant< NullState, ConfigState, SyncState, OnlineState> m_vState;
			State *m_pclCurrentState = nullptr;			

			TimeoutController	m_clTimeoutController;

			std::uint16_t		m_uRemoteFreeRam = UINT16_MAX;

			//
			//
			//
			//
			//						
			std::list<std::shared_ptr<detail::NetworkTaskImpl>> m_lstTasks;				

			/**
			Registered is a device that is stored on config.

			Devices that contact the Broker, but are not in the config files, are marked as unregistered

			*/				
			bool					m_fRegistered;		
	};

}