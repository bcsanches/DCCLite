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

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include <dcclite/Benchmark.h>
#include <dcclite/Socket.h>

#include "IDccLiteService.h"
#include "Device.h"
#include "IDevice.h"
#include "NetworkDeviceEventLog.h"
#include "NetworkDeviceTasks.h"
#include "PinManager.h"

#include "sys/Thinker.h"
#include "sys/Timeouts.h"

namespace dcclite::broker::exec::dcc
{ 	
	class ServoTurnoutDecoder;

	class NetworkDevice: public Device, private INetworkDevice_DecoderServices, private detail::INetworkDevice_TaskServices, private INetworkDevice_TaskProvider
	{
		public:
			enum class Status: uint8_t
			{
				OFFLINE,
				CONNECTING,
				ONLINE			
			};

		private:
			class FlowRateManager
			{
				public:
					inline void OnPacketReceived(const dcclite::Packet &packet) noexcept
					{
						m_uBytesReceivedCount += packet.GetSize();
					}

					inline void OnPacketSent(const dcclite::Packet &packet) noexcept
					{
						m_uBytesSentCount += packet.GetSize();
					}

					inline void Serialize(dcclite::JsonOutputStream_t &stream)
					{
						stream.AddIntValue("bytesReceivedCount", m_uBytesReceivedCount);
						stream.AddIntValue("bytesSentCount", m_uBytesSentCount);
					}

				private:
					unsigned			m_uBytesReceivedCount = 0;
					unsigned			m_uBytesSentCount = 0;					
			};

			class NetworkDeviceServiceWrapper
			{
				public:
					inline NetworkDeviceServiceWrapper(IDccLite_NetworkDeviceServices &dccService) :
						m_rclDccService{ dccService },						
						m_clFlowRateNotifyThinker{"NetworkDeviceServiceWrapper::FlowRateNotify", THINKER_MF_LAMBDA(OnFlowRateNotifyThink)}
					{
						//empty
					}

					inline void SendPacket(NetworkDevice &owner, const dcclite::Packet &packet)
					{
						this->DoSendPacket(owner.GetRemoteAddress(), packet);

						this->NotifyFlowRateChange(owner, std::nullopt);
					}

					inline void SendPacket(NetworkDevice &owner, const dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
					{
						this->DoSendPacket(owner.GetRemoteAddress(), packet);

						this->NotifyFlowRateChange(owner, time);
					}

					inline void Block(NetworkDevice &owner)
					{
						m_rclDccService.NetworkDevice_Block(owner);
					}

					inline void UnregisterSession(NetworkDevice &owner, const dcclite::Guid &sessionToken)
					{
						m_rclDccService.NetworkDevice_UnregisterSession(owner, sessionToken);
					}

					inline void RegisterSession(NetworkDevice &owner, const dcclite::Guid &configToken)
					{
						m_rclDccService.NetworkDevice_RegisterSession(owner, configToken);
					}

					inline void DestroyUnregistered(NetworkDevice &owner)
					{
						m_rclDccService.NetworkDevice_DestroyUnregistered(owner);
					}

					inline void NotifyStateChange(NetworkDevice &owner, dcclite::broker::sys::ObjectManagerEvent::SerializeDeltaProc_t proc)
					{
						if(!m_fFlowRateChangePending)
							m_rclDccService.NetworkDevice_NotifyStateChange(owner, proc);
						else
						{
							//inject on the notify packet on the data...
							m_rclDccService.NetworkDevice_NotifyStateChange(owner, [this, proc](JsonOutputStream_t &stream)
								{
									proc(stream);
									this->m_clFlowRateManager.Serialize(stream);
								}
							);

							m_clFlowRateNotifyThinker.Cancel();
							m_fFlowRateChangePending = false;
						}
					}

					inline void NotifyReceivedPacket(NetworkDevice &owner, const dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
					{
						m_clFlowRateManager.OnPacketReceived(packet);
						this->NotifyFlowRateChange(owner, time);
					}

				private:
					inline void DoSendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet &packet)
					{
						m_rclDccService.NetworkDevice_SendPacket(destination, packet);
						m_clFlowRateManager.OnPacketSent(packet);
					}

					void DispatchFlowRateChangeNotify(NetworkDevice &owner, const dcclite::Clock::TimePoint_t time);

					void OnFlowRateNotifyThink(const dcclite::Clock::TimePoint_t time);

					void NotifyFlowRateChange(NetworkDevice &owner, std::optional<const dcclite::Clock::TimePoint_t> time);

				private:
					IDccLite_NetworkDeviceServices	&m_rclDccService;					

					sys::Thinker_t					m_clFlowRateNotifyThinker;

					FlowRateManager					m_clFlowRateManager;

					NetworkDevice					*m_pclThinkerDeviceOwner = nullptr;

					bool							m_fFlowRateChangePending = false;
			};

		public:
			NetworkDevice(RName name, sys::Broker &broker, IDccLite_NetworkDeviceServices &dccService, const rapidjson::Value &params);
			NetworkDevice(RName name, IDccLite_NetworkDeviceServices &dccService);

			NetworkDevice(const NetworkDevice &) = delete;
			NetworkDevice(NetworkDevice &&) = delete;

			~NetworkDevice() override;			

			void AcceptConnection(
				dcclite::Clock::TimePoint_t	time, 
				dcclite::NetworkAddress		remoteAddress, 
				dcclite::Guid				remoteSessionToken, 
				dcclite::Guid				remoteConfigToken, 
				const std::uint16_t			protocolVersion
			);

			void OnPacket(
				dcclite::Packet						&packet, 
				const dcclite::Clock::TimePoint_t	time, 
				const dcclite::MsgTypes				msgType, 
				const dcclite::NetworkAddress		remoteAddress, 
				const dcclite::Guid					remoteConfigToken
			);
					
			[[nodiscard]] inline const dcclite::Guid &GetConfigToken() noexcept
			{
				return m_guidConfigToken;
			}

			[[nodiscard]] uint8_t FindDecoderIndex(const Decoder &decoder) const override;

			[[nodiscard]] virtual Decoder &FindDecoder(RName name) const override;

			void ResetRemoteDevice();

			bool IsConnectionStable() const noexcept override;

			void DisconnectDevice();

			inline void Block()
			{
				m_clNetService.Block(*this);	
			}

			inline NetworkAddress GetRemoteAddress() const noexcept
			{
				return m_clRemoteAddress;
			}

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

			[[nodiscard]] INetworkDevice_TaskProvider *TryGetINetworkTaskProvider() noexcept override
			{
				return this;
			}

			[[nodiscard]] std::uint16_t GetProtocolVersion() const noexcept override
			{
				return m_uProtocolVersion;
			}						

			//
			//
			// Tasks
			//
			//			
			[[nodiscard]] std::shared_ptr<NetworkTask> StartDownloadEEPromTask(NetworkTask::IObserver *observer, DownloadEEPromTaskResult_t &resultsStorage) override;
			[[nodiscard]] std::shared_ptr<NetworkTask> StartServoTurnoutProgrammerTask(NetworkTask::IObserver *observer, Decoder &decoder) override;
			[[nodiscard]] std::shared_ptr<NetworkTask> StartDeviceRenameTask(NetworkTask::IObserver *observer, RName newName) override;
			[[nodiscard]] std::shared_ptr<NetworkTask> StartDeviceClearEEPromTask(NetworkTask::IObserver *observer) override;
			[[nodiscard]] std::shared_ptr<NetworkTask> StartDeviceNetworkTestTask(NetworkTask::IObserver *observer, std::chrono::milliseconds timeout = sys::TASK_NETWORK_TEST_DEFAULT_TIMEOUT) override;

		protected:
			void OnUnload() override;

			void CheckIfDecoderTypeIsAllowed(Decoder &decoder) override;
			[[nodiscard]] bool IsInternalDecoderAllowed() const noexcept override;

		private:
			//
			//
			//INetworkDevice_TaskServices
			//
			//

			void TaskServices_FillPacketHeader(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType) const noexcept override;

			void TaskServices_SendPacket(dcclite::Packet &packet) override;

			void TaskServices_Disconnect() override;			

			void TaskServices_ForgetTask(NetworkTask &task) override;

			////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////

			[[nodiscard]] bool CheckSessionConfig(const dcclite::Guid remoteConfigToken, const dcclite::NetworkAddress remoteAddress);
			
			[[nodiscard]] bool CheckSession(const dcclite::NetworkAddress remoteAddress);

			/**
			* This methods sets the state as offline, such as in cases when a connection is dropped
			* 
			* Also, if device is unregistered, the method maybe suicidal, device will be destroyed after going offline
			* 			
			*
			*/
			void GoOffline();			

			template <typename T, class... Args>
			void SetState(Args&&...args);
		
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

			//
			//
			//
			//
			//

			void SerializeFreeRam(dcclite::JsonOutputStream_t &stream) const;
			void SerializeConnectionStatus(dcclite::JsonOutputStream_t &stream) const;

		private:						
			PinManager				m_clPinManager;		

			//
			//
			//Remote Device Info				
			dcclite::Guid			m_guidSessionToken;

			dcclite::NetworkAddress	m_clRemoteAddress;

			//
			//
			//
			NetworkDeviceServiceWrapper m_clNetService;	

			friend struct OnPacketImpl;			

			struct State
			{
				virtual ~State() = default;

				void OnPacket(					
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

				sys::Thinker_t		m_clTimeoutThinker;

				BenchmarkLogger		m_clBenchmark;

				ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time);					

				void OnPacket(					
					dcclite::Packet &packet,
					const dcclite::Clock::TimePoint_t time,
					const dcclite::MsgTypes msgType,
					const dcclite::NetworkAddress remoteAddress,
					const dcclite::Guid remoteConfigToken
				);				

				[[nodiscard]] 
				const char *GetName() const override { return "ConfigState"; }

				private:				
					void SendDecoderConfigPacket(const size_t index, const dcclite::Clock::TimePoint_t time) const;
					void SendConfigStartPacket(const dcclite::Clock::TimePoint_t time) const;
					void SendConfigFinishedPacket(const dcclite::Clock::TimePoint_t time) const;

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
				);				

				[[nodiscard]] const char *GetName() const override { return "SyncState"; }

				private:
					void OnTimeout(const dcclite::Clock::TimePoint_t time);

					sys::Thinker_t		m_clTimeoutThinker;

					BenchmarkLogger		m_clBenchmark;
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
				);				

				[[nodiscard]] const char *GetName() const override { return "OnlineState"; }

				void OnChangeStateRequest(const Decoder &decoder);

				private:
					bool SendStateDelta(const bool sendSensorsState, const dcclite::Clock::TimePoint_t time, const std::string_view requester);

					void OnPingThink(const dcclite::Clock::TimePoint_t time);
					void OnStateDeltaThink(const dcclite::Clock::TimePoint_t time);									

					uint64_t			m_uLastReceivedStatePacketId = 0;

					uint64_t			m_uOutgoingStatePacketId = 0;

					sys::Thinker_t		m_clPingThinker;
					sys::Thinker_t		m_clSendStateDeltaThinker;

					BenchmarkLogger		m_clBenchmark;

					bool				m_fPendingPong = false;
					bool				m_fLostPingPacket = false;
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
					sys::Thinker_t	m_clThinker;

					NetworkDevice	&m_rclOwner;
			};


			//
			//
			//Connection state
		
			std::variant< std::monostate, ConfigState, SyncState, OnlineState> m_vState;			

			TimeoutController	m_clTimeoutController;

			std::uint16_t		m_uRemoteFreeRam = UINT16_MAX;
			std::uint16_t		m_uProtocolVersion = 0;
			
			Status				m_kStatus = Status::OFFLINE;

			/**
			Registered is a device that is stored on config.

			Devices that contact the Broker, but are not in the config files, are marked as unregistered

			*/
			bool				m_fRegistered;

			//
			//
			//
			//
			//						
			std::list<std::shared_ptr<detail::NetworkTaskImpl>> m_lstTasks;

			NetworkDeviceEventLog	m_clEventLog;			
	};
}
