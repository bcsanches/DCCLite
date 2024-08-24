// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NetworkDevice.h"

#include <magic_enum/magic_enum.hpp>

#include "../sys/Project.h"

#include "BitPack.h"
#include "IDccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "SensorDecoder.h"
#include "TurnoutDecoder.h"

namespace dcclite::broker
{

	using namespace std::chrono_literals;

	static auto constexpr TIMEOUT_TICKS = 10s;
	static auto constexpr CONFIG_RETRY_TIME = 100ms;
	static auto constexpr STATE_TIMEOUT = 250ms;

	static auto constexpr SYNC_TIMEOUT = 100ms;

	static auto constexpr PING_TIMEOUT = 4s;

	static uint32_t	g_u32TaskId = 0;


	class DevicePacket : public dcclite::Packet
	{
		public:
			DevicePacket(dcclite::MsgTypes msgType, const dcclite::Guid &sessionToken, const dcclite::Guid &configToken)
			{
				dcclite::PacketBuilder builder{ *this, msgType, sessionToken, configToken };
			}
	};

	//
	//
	//
	//
	//
	//

	NetworkDevice::TimeoutController::TimeoutController(NetworkDevice &owner):
		m_clThinker{ "NetworkDevice::TimeoutController", THINKER_MF_LAMBDA(OnThink)},
		m_rclOwner(owner)
	{
		//empty
	}

	void NetworkDevice::TimeoutController::Enable(const dcclite::Clock::TimePoint_t currentTime)
	{
		m_clThinker.Schedule(currentTime + TIMEOUT_TICKS);
	}

	void NetworkDevice::TimeoutController::Disable()
	{
		m_clThinker.Cancel();
	}

	void NetworkDevice::TimeoutController::OnThink(const dcclite::Clock::TimePoint_t time)
	{
		dcclite::Log::Warn("[Device::{}] [TimeoutController::OnThink] timeout", m_rclOwner.GetName());

		//
		//connection lost... 
		m_rclOwner.GoOffline();
	}

	//
	//
	// DEVICE CONSTRUCTION / DESTRUCTION
	//
	//



	NetworkDevice::NetworkDevice(RName name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project):
		Device(name, dccService, params, project),		
		m_clPinManager(DecodeBoardName(params["class"].GetString())),
		m_fRegistered(true),
		m_clTimeoutController{ *this }
	{
		this->Load();
	}


	NetworkDevice::NetworkDevice(RName name, IDccLite_DeviceServices &dccService, const Project &project) :
		Device(name, dccService, project),		
		m_clPinManager(ArduinoBoards::MEGA),		
		m_fRegistered(false),
		m_clTimeoutController{*this}
	{
		//empty
	}

	NetworkDevice::~NetworkDevice()
	{		
		this->Unload();
	}

	void NetworkDevice::CheckLoadedDecoder(Decoder &decoder)
	{
		if (!dynamic_cast<RemoteDecoder *>(&decoder))
			throw std::invalid_argument(fmt::format("[NetworkDevice::{}] [CheckLoadedDecoder] Decoder {} must be a RemoteDecoder subtype, but it is: {}", this->GetName(), decoder.GetName(), decoder.GetTypeName()));
	}

	bool NetworkDevice::IsInternalDecoderAllowed() const noexcept
	{
		return false;
	}

	void NetworkDevice::DisconnectDevice()
	{
		DevicePacket pkt{ dcclite::MsgTypes::DISCONNECT, m_SessionToken, m_ConfigToken };

		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

		dcclite::Log::Warn("[Device::{}] [Disconnect] Sent disconnect packet", this->GetName());

		this->GoOffline();
	}

	void NetworkDevice::OnUnload()
	{
		Device::OnUnload();

		//
		//During unload we go to a inconsistent state, so we force a disconnect so we ignore all remote device data
		//We will need to go throught all the reconnect process

		if (m_kStatus == Status::OFFLINE)
			return;

		this->DisconnectDevice();		
	}

	//
	//
	// Base STATE
	//
	//

	void NetworkDevice::State::OnPacket(		
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken
	)
	{
		dcclite::Log::Error("[Device::{}] [{}::OnPacket] Unexpected msg type: {} - {}", this->GetName(), m_rclSelf.GetName(), (int)msgType, dcclite::MsgName(msgType));
	}

	//
	//
	// ConfigState
	//
	//

	NetworkDevice::ConfigState::ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time):
		State(self),
		m_clTimeoutThinker{"NetworkDevice::ConfigState::TimeoutThinker", THINKER_MF_LAMBDA(OnTimeout)}
	{
		this->m_vecAcks.resize(self.m_vecDecoders.size());

		m_clTimeoutThinker.Schedule(time + CONFIG_RETRY_TIME);

		this->SendConfigStartPacket();

		for (size_t i = 0, sz = self.m_vecDecoders.size(); i < sz; ++i)
		{
			this->SendDecoderConfigPacket(i);
		}
	}

	void NetworkDevice::ConfigState::SendConfigStartPacket() const
	{
		DevicePacket pkt{ dcclite::MsgTypes::CONFIG_START, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);
	}

	void NetworkDevice::ConfigState::SendDecoderConfigPacket(const size_t index) const
	{
		DevicePacket pkt{ dcclite::MsgTypes::CONFIG_DEV, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };
		pkt.Write8(static_cast<uint8_t>(index));

		static_cast<RemoteDecoder *>(m_rclSelf.m_vecDecoders[index])->WriteConfig(pkt);

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);
	}

	void NetworkDevice::ConfigState::SendConfigFinishedPacket() const
	{
		DevicePacket pkt{ dcclite::MsgTypes::CONFIG_FINISHED, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

		pkt.Write8(static_cast<uint8_t>(m_rclSelf.m_vecDecoders.size()));

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);
	}	

	void NetworkDevice::ConfigState::OnPacket_ConfigAck(
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken)
	{
		if (!m_rclSelf.CheckSession(remoteAddress))
			return;

		auto seq = packet.Read<uint8_t>();

		if (seq >= m_vecAcks.size())
		{
			dcclite::Log::Error("[Device::{}] [{}::OnPacket_ConfigAck] config out of sync, dropping connection", m_rclSelf.GetName(), this->GetName());

			m_rclSelf.GoOffline();

			return;
		}		

		m_rclSelf.m_clTimeoutController.Enable(time);

		//only increment seq count if m_vecAcks[seq] is not set yet, so we handle duplicate packets
		m_uSeqCount += m_vecAcks[seq] == false;
		m_vecAcks[seq] = true;

		m_clTimeoutThinker.Schedule(time + CONFIG_RETRY_TIME);		

		dcclite::Log::Info("[Device::{}] [{}::OnPacket_ConfigAck] Config ACK {} - {}", m_rclSelf.GetName(), this->GetName(), seq, m_rclSelf.m_vecDecoders[seq]->GetName());

		if (m_uSeqCount == m_vecAcks.size())
		{
			dcclite::Log::Info("[Device::{}] [{}::OnPacket_ConfigAck] Config Finished, configured {} decoders", m_rclSelf.GetName(), this->GetName(), m_uSeqCount);

			this->SendConfigFinishedPacket();
		}
	}

	void NetworkDevice::ConfigState::OnPacket_ConfigFinished(		
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken
	)
	{
		if (!m_rclSelf.CheckSession(remoteAddress))
			return;

		dcclite::Log::Info("[Device::{}] [{}::OnPacket_ConfigFinished] Config Finished, device is ready", m_rclSelf.GetName(), this->GetName());

		m_rclSelf.m_clTimeoutController.Enable(time);

		m_rclSelf.GotoSyncState();
	}

	void NetworkDevice::ConfigState::OnPacket(	
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken)
	{
		switch (msgType)
		{
			case dcclite::MsgTypes::CONFIG_ACK:
				this->OnPacket_ConfigAck(packet, time, msgType, remoteAddress, remoteConfigToken);
				break;

			case dcclite::MsgTypes::CONFIG_FINISHED:
				this->OnPacket_ConfigFinished(packet, time, msgType, remoteAddress, remoteConfigToken);
				break;

			default:
				State::OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);
				break;
		}
	}

	void NetworkDevice::ConfigState::OnTimeout(const dcclite::Clock::TimePoint_t time)
	{		
		//we havent received ack for some time, wake up the device (timeout keeps counting)
		m_clTimeoutThinker.Schedule(time + CONFIG_RETRY_TIME);

		//go thought all the decoders and check what does not have an ACK
		int pos = 0, packetCount = 0;
		for (const bool &acked : m_vecAcks)
		{
			if (!acked)
			{
				if (pos == 0)
				{
					//when config 0 is not received, this could also means that CONFIG_START was not received by tge remote, so we send it again
					this->SendConfigStartPacket();
				}

				dcclite::Log::Warn(
					"[Device::{}] [{}::OnTimeout] retrying config for device {} at {}", 
					m_rclSelf.GetName(), 
					this->GetName(), 
					m_rclSelf.m_vecDecoders[pos]->GetName(), 
					pos
				);

				this->SendDecoderConfigPacket(pos);

				++packetCount;

				//too many packets? Wait a little bit
				if (packetCount == 3)
					break;
			}

			++pos;
		}

		if (packetCount == 0)
		{
			//remote device already acked all decoders, but not acked the config finished, so, send it again
			dcclite::Log::Warn("[Device::{}] [{}::Update] retrying config finished for remote device", m_rclSelf.GetName(), this->GetName());
			this->SendConfigFinishedPacket();
		}
	}

	//
	//
	// SyncState
	//
	//

	NetworkDevice::SyncState::SyncState(NetworkDevice &self):
		State{ self },
		m_clTimeoutThinker{"NetworkDevice::SyncState::TimeoutThinker", THINKER_MF_LAMBDA(OnTimeout)}
	{
		//force it to run ASAP
		m_clTimeoutThinker.Schedule({});
	}

	void NetworkDevice::SyncState::OnPacket(		
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken
	)
	{
		//we simple ignore state msgs as the device client does not keeps waiting for sync messages
		if ((msgType == dcclite::MsgTypes::STATE) || (msgType == dcclite::MsgTypes::RAM_DATA))
		{
			//dcclite::Log::Warn("[{}::Device::SyncState::OnPacket] FIXME change client to stop sending STATE msg on sync state", self.GetName());
			return;
		}

		if (msgType == dcclite::MsgTypes::CONFIG_FINISHED)
		{
			dcclite::Log::Warn("[NetworkDevice::{}] [SyncState::OnPacket] Got a late CONFIG_FINISHED packet, ignoring", this->m_rclSelf.GetName());

			return;
		}

		if (msgType != dcclite::MsgTypes::SYNC)
		{
			State::OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);

			return;
		}

		if (!m_rclSelf.CheckSessionConfig(remoteConfigToken, remoteAddress))
			return;

		m_rclSelf.m_clTimeoutController.Enable(time);

		dcclite::StatesBitPack_t changedStates;
		dcclite::StatesBitPack_t states;

		packet.ReadBitPack(changedStates);
		packet.ReadBitPack(states);

		for (unsigned i = 0; i < changedStates.size(); ++i)
		{
			if (!changedStates[i])
				continue;

			auto state = states[i] ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;
			auto remoteDecoder = static_cast<RemoteDecoder *>(m_rclSelf.m_vecDecoders[i]);
			remoteDecoder->SyncRemoteState(state);

			if (remoteDecoder->IsOutputDecoder())
			{
				auto *outputDecoder = static_cast<OutputDecoder *>(remoteDecoder);

				//if after a sync, the requested state changes, we toggle it, so they are synced
				//Hack?
				if (outputDecoder->GetPendingStateChange())
				{
					outputDecoder->ToggleState("OnPacket_Sync");
				}
			}
		}

		dcclite::Log::Info("[Device::{}] [SyncState::OnPacket] Sync OK", m_rclSelf.GetName());

		m_rclSelf.GotoOnlineState(time);
	}

	void NetworkDevice::SyncState::OnTimeout(const dcclite::Clock::TimePoint_t time)
	{
		assert(m_rclSelf.m_kStatus == Status::CONNECTING);
				
		dcclite::Log::Info("[Device::{}] [SyncState::OnTimeout] request sent", m_rclSelf.GetName());

		DevicePacket pkt{ dcclite::MsgTypes::SYNC, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

		m_clTimeoutThinker.Schedule(time + SYNC_TIMEOUT);		
	}

	//
	//
	// OnlineState
	//
	//

	NetworkDevice::OnlineState::OnlineState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time):
		State{ self },
		m_clPingThinker{"NetworkDevice::OnlineState::m_clPingThinker", THINKER_MF_LAMBDA(OnPingThink)},
		m_clSendStateDeltaThinker{"NetworkDevice::OnlineState::m_clSendStateDeltaThinker", THINKER_MF_LAMBDA(OnStateDeltaThink)}
	{		
		m_clPingThinker.Schedule(time + PING_TIMEOUT);

		//force it to send states ASAP
		m_clSendStateDeltaThinker.Schedule({});

		m_rclSelf.m_clTimeoutController.Enable(time);		
	}

	bool NetworkDevice::OnlineState::SendStateDelta(const bool sendSensorsState, const dcclite::Clock::TimePoint_t time, const std::string_view requester)
	{
		dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> states;
		dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> changedStates;

		bool stateChanged = false;
		bool sensorDetected = false;		

		const unsigned numDecoders = static_cast<unsigned>(std::min(m_rclSelf.m_vecDecoders.size(), size_t{ dcclite::MAX_DECODERS_STATES_PER_PACKET }));
		for (unsigned i = 0; i < numDecoders; ++i)
		{
			auto *decoder = static_cast<RemoteDecoder *>(m_rclSelf.m_vecDecoders[i]);
			if (!decoder)
				continue;

			dcclite::DecoderStates state;

			if (decoder->IsOutputDecoder())
			{
				auto *outputDecoder = static_cast<OutputDecoder *>(decoder);

				auto stateChange = outputDecoder->GetPendingStateChange();
				if (!stateChange)
					continue;				

				//dcclite::Log::Debug("SendStateDelta: change for {}", outputDecoder->GetName());

				state = stateChange.value();

				//mark on bit vector that this decoder has a change
				changedStates.SetBit(i);

				//Send down the decoder state
				states.SetBitValue(i, state == dcclite::DecoderStates::ACTIVE);

				stateChanged = true;
			}
			else if (decoder->IsInputDecoder())
			{
				sensorDetected = true;
			}
		}

		//if we have sensors and any state change on output decoders, we will send the sensors states anyway....
		//Otherwise, if the caller asked for sending sensorState and we have sensors... send it
		//We do on two steps, so anytime we send any output change, we also send the sensors state on the same packet
		if (sensorDetected && (stateChanged || sendSensorsState))
		{
			//force it to true for cases where statechanged is false (meaning that no output changed state) 
			//and sendSensorState is true....
			stateChanged = true;			

			for (unsigned i = 0; i < numDecoders; ++i)
			{
				auto *decoder = static_cast<RemoteDecoder *>(m_rclSelf.m_vecDecoders[i]);
				if (!decoder)
					continue;

				dcclite::DecoderStates state;

				if (!decoder->IsInputDecoder())
					continue;

				state = decoder->GetState();

				//mark on bit vector that this decoder has a change
				changedStates.SetBit(i);

				//Send down the decoder state
				states.SetBitValue(i, state == dcclite::DecoderStates::ACTIVE);
			}
		}

		//nothing to do?
		if (!stateChanged)
			return false;

		dcclite::Log::Debug("[Device::{}] [{}::OnPacket] Sending state - requester {}", m_rclSelf.GetName(), this->GetName(), requester);
					
		DevicePacket pkt{ dcclite::MsgTypes::STATE, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

		pkt.Write64(++m_uOutgoingStatePacketId);
		pkt.Write(changedStates);
		pkt.Write(states);

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);		

		return true;		
	}


	void NetworkDevice::OnlineState::OnPacket(		
		dcclite::Packet &packet,
		const dcclite::Clock::TimePoint_t time,
		const dcclite::MsgTypes msgType,
		const dcclite::NetworkAddress remoteAddress,
		const dcclite::Guid remoteConfigToken
	)
	{
		if (!m_rclSelf.CheckSessionConfig(remoteConfigToken, remoteAddress))
			return;
		
		m_rclSelf.m_clTimeoutController.Enable(time);		

		if (msgType == dcclite::MsgTypes::MSG_PONG)
		{			
			//Just ignore it...
			//any packet updates timeout do it
			//m_rclSelf.PostponeTimeout(time);

			//dcclite::Log::Debug("[{}::Device::OnPacket] pong", m_rclSelf.GetName());

			return;
		}

		if (msgType == dcclite::MsgTypes::TASK_DATA)
		{
			auto taskId = packet.Read<uint32_t>();			
			for (auto &task : m_rclSelf.m_lstTasks)
			{
				if (task->GetTaskId() == taskId)
				{
					task->OnPacket(packet, time);
					return;
				}
			}
			
			dcclite::Log::Warn("[Device::{}] [{}::OnPacket] task data, but no task running or task not found for id {}", m_rclSelf.GetName(), this->GetName(), taskId);

			return;
		}

		if (msgType == dcclite::MsgTypes::RAM_DATA)
		{			
			m_rclSelf.m_uRemoteFreeRam = packet.Read<uint16_t>();

			dcclite::Log::Warn("[Device::{}] [{}::OnPacket] Got RAM update: {}", m_rclSelf.GetName(), this->GetName(), m_rclSelf.m_uRemoteFreeRam);

			//Send the same packet back, so the remote device can ACK we got it
			DevicePacket pkt{ dcclite::MsgTypes::RAM_DATA, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

			pkt.Write16(m_rclSelf.m_uRemoteFreeRam);			

			//dispatch packet
			m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

			//let other systems know that our internal state changed...
			m_rclSelf.m_clDccService.Device_NotifyStateChange(m_rclSelf);

			return;
		}

		//a late sync message arrived?
		if (msgType == dcclite::MsgTypes::SYNC)
		{
			//ignore
			Log::Trace("[NetworkDevice::{}] [OnlineState::OnPacket] Got late SYNC message, ignoring", this->m_rclSelf.GetName());

			return;

		}

		if (msgType != dcclite::MsgTypes::STATE)
		{
			State::OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);

			return;
		}

		const auto sequenceCount = packet.Read<uint64_t>();

		//discard old packets
		if (sequenceCount < m_uLastReceivedStatePacketId)
			return;

		m_uLastReceivedStatePacketId = sequenceCount;

		dcclite::StatesBitPack_t changedStates;
		dcclite::StatesBitPack_t states;

		packet.ReadBitPack(changedStates);
		packet.ReadBitPack(states);			

		bool sensorStateRefresh = false;
		
		for (unsigned i = 0; i < changedStates.size(); ++i)
		{
			if (!changedStates[i])
				continue;

			auto state = states[i] ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;

			auto remoteDecoder = static_cast<RemoteDecoder *>(m_rclSelf.m_vecDecoders[i]);
			remoteDecoder->SyncRemoteState(state);

			/**

			The remote device sent the state of any input (sensors)

			So if we received any sensor state, we send back to the client our current state so it can ACK our current state
			*/

			sensorStateRefresh = remoteDecoder->IsInputDecoder() || sensorStateRefresh;			
		}		

		if (sensorStateRefresh)
		{
			//
			//Send it back to the device, so it can ACK that we got the last state
			SendStateDelta(sensorStateRefresh, time, "OnPacket");
		}		
	}

	void NetworkDevice::OnlineState::OnPingThink(const dcclite::Clock::TimePoint_t time)
	{
		m_clPingThinker.Schedule(time + PING_TIMEOUT);

		DevicePacket pkt{ dcclite::MsgTypes::MSG_PING, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };
		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);		
	}

	void NetworkDevice::OnlineState::OnChangeStateRequest(const Decoder &decoder)
	{		
		//Run this ASAP
		m_clSendStateDeltaThinker.Schedule({});
	}

	void NetworkDevice::OnlineState::OnStateDeltaThink(const dcclite::Clock::TimePoint_t time)
	{
		if (!this->SendStateDelta(false, time, "OnStateDeltaThink"))
		{
			//no state sent? nothing else to do here
			return;
		}

		/*
		* 
		* State sent, so we schedule to send it again for covering packet lost cases
		* 
		* If  the state arrive on the other side, we will get a reply that will sync our state,
		* and hopefully, the lastStateSent will be the same, so the send will fail and we will not schedule it again		
		*/
		m_clSendStateDeltaThinker.Schedule(time + STATE_TIMEOUT);
	}

	//
	//
	//
	//
	//


	void NetworkDevice::Decoder_OnChangeStateRequest(const Decoder &decoder) noexcept
	{
		if (auto *onlineState = std::get_if<OnlineState>(&m_vState))
			onlineState->OnChangeStateRequest(decoder);
	}

	void NetworkDevice::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Device::Serialize(stream);

		stream.AddBool("registered", m_fRegistered);
		stream.AddStringValue("configToken", dcclite::GuidToString(m_ConfigToken));
		stream.AddStringValue("sessionToken", dcclite::GuidToString(m_SessionToken));
		stream.AddStringValue("remoteAddress", m_RemoteAddress.GetIpString());
		stream.AddIntValue("connectionStatus", static_cast<int>(m_kStatus));
		stream.AddIntValue("protocolVersion", m_uProtocolVersion);
		stream.AddIntValue("freeRam", m_uRemoteFreeRam);

		m_clPinManager.Serialize(stream);
	}

	void NetworkDevice::GoOffline()
	{
		if (m_kStatus == Status::OFFLINE)
			return;

		m_kStatus = Status::OFFLINE;

		m_clDccService.Device_UnregisterSession(*this, m_SessionToken);
		m_SessionToken = dcclite::Guid{};

		this->ClearState();
		this->AbortPendingTasks();

		m_clTimeoutController.Disable();

		dcclite::Log::Warn("[Device::{}] [GoOffline] Is OFFLINE", this->GetName());
		m_clDccService.Device_NotifyStateChange(*this);

		if (m_fRegistered)
			return;
		
		//kill ourselves...
		m_clDccService.Device_DestroyUnregistered(*this);
	}


	void NetworkDevice::AcceptConnection(
		const dcclite::Clock::TimePoint_t	time, 
		const dcclite::NetworkAddress		remoteAddress, 
		const dcclite::Guid					remoteSessionToken, 
		const dcclite::Guid					remoteConfigToken, 
		const std::uint16_t					protocolVersion
	)
	{
		if (m_kStatus != Status::OFFLINE)
		{
			dcclite::Log::Error("[Device::{}] [AcceptConnection] Already connected, cannot accept request from {}", this->GetName(), remoteAddress);

			return;
		}

		m_uProtocolVersion = protocolVersion;
		m_RemoteAddress = remoteAddress;
		m_SessionToken = dcclite::GuidCreate();

		m_kStatus = Status::CONNECTING;

		m_clDccService.Device_RegisterSession(*this, m_SessionToken);

		m_clTimeoutController.Enable(time);

		dcclite::Log::Info("[Device::{}] [AcceptConnection] Is connecting", this->GetName());

		//this->RefreshTimeout(time);

		//Is device config expired?
		if (remoteConfigToken != m_ConfigToken)
		{
			dcclite::Log::Info("[Device::{}] [AcceptConnection] Started configuring for token {}", this->GetName(), m_ConfigToken);

			this->GotoConfigState(time);

			dcclite::Log::Info("[Device::{}] [AcceptConnection] config data sent", this->GetName());
		}
		else
		{
			//device config is fine...
			dcclite::Log::Info("[Device::{}] [AcceptConnection] Accepted connection {} {}", this->GetName(), remoteAddress, m_ConfigToken);

			//tell device that connection was accepted
			DevicePacket pkt{ dcclite::MsgTypes::ACCEPTED, m_SessionToken, m_ConfigToken };
			m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

			//now sync it
			this->GotoSyncState();
		}

		m_clDccService.Device_NotifyStateChange(*this);
	}

	bool NetworkDevice::CheckSessionConfig(dcclite::Guid remoteConfigToken, dcclite::NetworkAddress remoteAddress)
	{
		if (!this->CheckSession(remoteAddress))
			return false;

		if (remoteConfigToken != m_ConfigToken)
		{
			dcclite::Log::Warn("[Device::{}] [CheckSessionConfig] Received packet from invalid config...", this->GetName());

			return false;
		}

		return true;
	}

	bool NetworkDevice::CheckSession(dcclite::NetworkAddress remoteAddress)
	{
		if (m_kStatus == Status::OFFLINE)
		{
			dcclite::Log::Error("[Device::{}] [CheckSession] got packet from disconnected device", this->GetName());

			return false;
		}

		if (remoteAddress != m_RemoteAddress)
		{
			dcclite::Log::Warn("[Device::{}] [CheckSession] Updating remote address, session valid", this->GetName());

			m_RemoteAddress = remoteAddress;
		}

		return true;
	}

	void NetworkDevice::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time, const dcclite::MsgTypes msgType, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteConfigToken)
	{
		if (!m_pclCurrentState)
		{
			dcclite::Log::Error("[Device::{}] [OnPacket] Cannot process packet on Offline mode, packet: {}", this->GetName(), dcclite::MsgName(msgType));

			return;
		}		

		m_pclCurrentState->OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);
	}

	void NetworkDevice::ClearState()
	{
		m_vState = std::monostate{};
		m_pclCurrentState = nullptr;
	}

	template <typename T, class... Args>
	void NetworkDevice::SetState(Args&&...args)
	{
		m_vState.emplace<T>(*this, args...);
		m_pclCurrentState = &std::get<T>(m_vState);
	}

	void NetworkDevice::GotoSyncState()
	{
		this->SetState<SyncState>();

		dcclite::Log::Trace("[Device::{}] [GotoSyncState] Entered", this->GetName());
	}

	void NetworkDevice::GotoOnlineState(const dcclite::Clock::TimePoint_t time)
	{
		m_kStatus = Status::ONLINE;

		this->SetState<OnlineState>(time);

		dcclite::Log::Trace("[Device::{}] [GotoOnlineState] Entered", this->GetName());
		m_clDccService.Device_NotifyStateChange(*this);
	}

	void NetworkDevice::GotoConfigState(const dcclite::Clock::TimePoint_t time)
	{
		this->SetState<ConfigState>(time);

		dcclite::Log::Trace("[Device::{}] [GotoConfigState] Entered", this->GetName());
	}	

	bool NetworkDevice::IsConnectionStable() const noexcept
	{
		//
		//make sure we are connected and on online state (sync and config are not valids)
		return std::holds_alternative<NetworkDevice::OnlineState>(m_vState);		
	}

	Decoder &NetworkDevice::FindDecoder(RName name) const
	{		
		for (size_t i = 0, len = m_vecDecoders.size(); i < len; ++i)
		{
			if (m_vecDecoders[i]->GetName() == name)
				return *m_vecDecoders[i];
		}

		throw std::out_of_range(fmt::format("[Device::{}] [FindDecoder] Decoder {} not found", this->GetName(), name));
	}

	uint8_t NetworkDevice::FindDecoderIndex(const Decoder &decoder) const
	{
		assert(m_vecDecoders.size() < 255);

		for (size_t i = 0, len = m_vecDecoders.size(); i < len; ++i)
		{
			if (m_vecDecoders[i] == &decoder)
				return static_cast<uint8_t>(i);
		}

		throw std::out_of_range(fmt::format("[Device::{}] [FindDecoderIndex] Decoder {} not found", this->GetName(), decoder.GetName()));
	}

	void NetworkDevice::TaskServices_FillPacketHeader(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType) const noexcept
	{
		PacketBuilder builder{ packet, MsgTypes::TASK_REQUEST, m_SessionToken, m_ConfigToken };

		packet.Write8(static_cast<uint8_t>(taskType));
		packet.Write32(taskId);
	}

	void NetworkDevice::TaskServices_SendPacket(dcclite::Packet &packet)
	{
		m_clDccService.Device_SendPacket(m_RemoteAddress, packet);
	}

	void NetworkDevice::TaskServices_ForgetTask(NetworkTask &task)
	{
		auto it = std::find_if(m_lstTasks.begin(), m_lstTasks.end(), [&task](std::shared_ptr<detail::NetworkTaskImpl> &ptr) { return ptr.get() == &task; });

		//
		//It may happens that a task may finishes and removes itself and a task owner ask its to stop after it... so ignore...
		if (it == m_lstTasks.end())
			return;

		m_lstTasks.erase(it);
	}	

	void NetworkDevice::TaskServices_Disconnect()
	{
		this->DisconnectDevice();
	}

	void NetworkDevice::AbortPendingTasks()
	{
		while (!m_lstTasks.empty())
		{
			auto t = m_lstTasks.front();
			m_lstTasks.pop_front();

			t->Abort();

		}		
	}

	std::shared_ptr<NetworkTask> NetworkDevice::StartDownloadEEPromTask(NetworkTask::IObserver *observer, DownloadEEPromTaskResult_t &resultsStorage)
	{		
		if (!this->IsConnectionStable())		
			throw std::runtime_error(fmt::format("[NetworkDevice::{}] [StartDownloadEEPromTask] Cannot start task without a connectd device", this->GetName()));

		auto task = detail::StartDownloadEEPromTask(*this, ++g_u32TaskId, observer, resultsStorage);								

		m_lstTasks.push_back(task);		

		return task;
	}

	std::shared_ptr<NetworkTask> NetworkDevice::StartServoTurnoutProgrammerTask(NetworkTask::IObserver *observer, RName servoDecoderName)
	{		
		if (!this->IsConnectionStable())
			throw std::runtime_error(fmt::format("[NetworkDevice::{}] [StartServoTurnoutProgrammerTask] Cannot start task without a connectd device", this->GetName()));

		auto obj = this->TryResolveChild(servoDecoderName);
		if (!obj)
			throw std::invalid_argument(fmt::format("[NetworkDevice::{}] [StartDownloadEEPromTask] Servo decoder {} not found", this->GetName(), servoDecoderName));

		auto servoTurnout = dynamic_cast<ServoTurnoutDecoder *>(obj);
		if(!servoTurnout)
			throw std::invalid_argument(fmt::format("[NetworkDevice::{}] [StartDownloadEEPromTask] Servo decoder {} is not a ServoTurnoutDecoder", this->GetName(), servoDecoderName));
		
		auto task = detail::StartServoTurnoutProgrammerTask(*this, ++g_u32TaskId, observer, *servoTurnout);
		
		m_lstTasks.push_back(task);		

		return task;
	}

	std::shared_ptr<NetworkTask> NetworkDevice::StartDeviceRenameTask(NetworkTask::IObserver *observer, RName newName)
	{		
		auto task = detail::StartDeviceRenameTask(*this, ++g_u32TaskId, observer, newName);

		m_lstTasks.push_back(task);

		return task;
	}

	std::shared_ptr<NetworkTask> NetworkDevice::StartDeviceClearEEPromTask(NetworkTask::IObserver *observer)
	{
		auto task = detail::StartDeviceClearEEPromTask(*this, ++g_u32TaskId, observer);

		m_lstTasks.push_back(task);

		return task;
	}
}
