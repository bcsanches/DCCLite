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

#include <magic_enum.hpp>

#include "BitPack.h"
#include "IDccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "Project.h"
#include "SensorDecoder.h"
#include "TurnoutDecoder.h"

namespace dcclite::broker
{

	using namespace std::chrono_literals;

	static auto constexpr TIMEOUT = 10s;
	static auto constexpr CONFIG_RETRY_TIME = 100ms;
	static auto constexpr STATE_TIMEOUT = 250ms;

	static auto constexpr SYNC_TIMEOUT = 100ms;

	static auto constexpr PING_TIMEOUT = 1s;

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
	// DEVICE CONSTRUCTION / DESTRUCTION
	//
	//

	NetworkDevice::NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project):
		Device(std::move(name), dccService, params, project),
		m_clTimeoutThinker(THINKER_MF_LAMBDA(OnTimeoutThink)),
		m_clPinManager(DecodeBoardName(params["class"].GetString())),
		m_fRegistered(true)
	{
		this->Load();
	}


	NetworkDevice::NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const Project &project) :
		Device(std::move(name), dccService, project),
		m_clTimeoutThinker(THINKER_MF_LAMBDA(OnTimeoutThink)),
		m_clPinManager(ArduinoBoards::MEGA),		
		m_fRegistered(false)
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
			throw std::invalid_argument(fmt::format("[NetworkDevice::CheckLoadedDecoder] Decoder {} must be a RemoteDecoder subtype, but it is: {}", decoder.GetName(), decoder.GetTypeName()));
	}

	void NetworkDevice::OnUnload()
	{
		Device::OnUnload();

		//
		//During unload we go to a inconsistent state, so we force a disconnect so we ignore all remote device data
		//We will need to go throught all the reconnect process

		this->Disconnect();
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
		dcclite::Log::Error("[{}::Device::{}::OnPacket] Unexpected msg type: {}", m_rclSelf.GetName(), this->GetName(), dcclite::MsgName(msgType));
	}

	//
	//
	// ConfigState
	//
	//

	NetworkDevice::ConfigState::ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time):
		State(self)
	{
		this->m_vecAcks.resize(self.m_vecDecoders.size());
		this->m_RetryTime = time + CONFIG_RETRY_TIME;

		this->SendConfigStartPacket();

		for (size_t i = 0, sz = self.m_vecDecoders.size(); i < sz; ++i)
		{
			this->SendDecoderConfigPacket(i);
		}
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

	void NetworkDevice::ConfigState::SendConfigStartPacket() const
	{
		DevicePacket pkt{ dcclite::MsgTypes::CONFIG_START, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

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
			dcclite::Log::Error("[{}::Device::OnPacket_ConfigAck] config out of sync, dropping connection", m_rclSelf.GetName());

			m_rclSelf.GoOffline();

			return;
		}

		//only increment seq count if m_vecAcks[seq] is not set yet, so we handle duplicate packets
		m_uSeqCount += m_vecAcks[seq] == false;
		m_vecAcks[seq] = true;

		m_RetryTime = time + CONFIG_RETRY_TIME;

		dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config ACK {} - {}", m_rclSelf.GetName(), seq, m_rclSelf.m_vecDecoders[seq]->GetName());

		if (m_uSeqCount == m_vecAcks.size())
		{
			dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config Finished, configured {} decoders", m_rclSelf.GetName(), m_uSeqCount);

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

		dcclite::Log::Info("[{}::Device::OnPacket_ConfigFinished] Config Finished, device is ready", m_rclSelf.GetName());

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

	void NetworkDevice::ConfigState::Update(const dcclite::Clock::TimePoint_t time)
	{
		//should retry sending config packets?	
		if (m_RetryTime > time)
			return;

		//we havent received ack for some time, wake up the device (timeout keeps counting)

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

				dcclite::Log::Warn("[{}::Device::Update] retrying config for device {} at {}", m_rclSelf.GetName(), m_rclSelf.m_vecDecoders[pos]->GetName(), pos);
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
			dcclite::Log::Warn("[{}::Device::Update] retrying config finished for remote device", m_rclSelf.GetName());
			this->SendConfigFinishedPacket();
		}
	}

	//
	//
	// SyncState
	//
	//

	NetworkDevice::SyncState::SyncState(NetworkDevice &self):
		State{ self }
	{
		//empty
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
		if (msgType == dcclite::MsgTypes::STATE)
		{
			//dcclite::Log::Warn("[{}::Device::SyncState::OnPacket] FIXME change client to stop sending STATE msg on sync state", self.GetName());
			return;
		}

		if (msgType != dcclite::MsgTypes::SYNC)
		{
			State::OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);

			return;
		}

		if (!m_rclSelf.CheckSessionConfig(remoteConfigToken, remoteAddress))
			return;

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

		dcclite::Log::Info("[{}::Device::SyncState::OnPacket] Sync OK", m_rclSelf.GetName());

		m_rclSelf.GotoOnlineState(time);
	}

	void NetworkDevice::SyncState::Update(const dcclite::Clock::TimePoint_t time)
	{
		assert(m_rclSelf.m_kStatus == Status::CONNECTING);

		//too soon?
		if (m_SyncTimeout > time)
			return;

		dcclite::Log::Info("[{}::Device::SyncState::Update] request sent", m_rclSelf.GetName());

		DevicePacket pkt{ dcclite::MsgTypes::SYNC, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

		m_SyncTimeout = time + SYNC_TIMEOUT;
	}

	//
	//
	// OnlineState
	//
	//

	NetworkDevice::OnlineState::OnlineState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time):
		State{ self },
		m_clPingThinker{THINKER_MF_LAMBDA(OnPingThink)}
	{
		m_tLastStateSent.ClearAll();
		m_tLastStateSentTimeout = {};

		m_clPingThinker.SetNext(time + PING_TIMEOUT);
		m_rclSelf.PostponeTimeout(time);
	}

	void NetworkDevice::OnlineState::SendStateDelta(const bool sendSensorsState, const dcclite::Clock::TimePoint_t time)
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

				state = decoder->GetRemoteState();

				//mark on bit vector that this decoder has a change
				changedStates.SetBit(i);

				//Send down the decoder state
				states.SetBitValue(i, state == dcclite::DecoderStates::ACTIVE);
			}
		}

		if (stateChanged)
		{			
			if ((m_tLastStateSent == states) && (time < m_tLastStateSentTimeout))
			{				
				return;
			}				

			DevicePacket pkt{ dcclite::MsgTypes::STATE, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };

			pkt.Write64(++m_uOutgoingStatePacketId);
			pkt.Write(changedStates);
			pkt.Write(states);

			m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

			m_tLastStateSentTimeout = time + STATE_TIMEOUT;
			m_tLastStateSent = states;

			//Log::Debug("[NetworkDevice::OnlineState::SendStateDelta] Sent {} sensor {}", m_uOutgoingStatePacketId, sendSensorsState);
		}
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

		m_rclSelf.PostponeTimeout(time);

		if (msgType == dcclite::MsgTypes::MSG_PONG)
		{
			//DevicePacket pkt{ dcclite::MsgTypes::MSG_PONG, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };
			//m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

			//any packet do it
			//m_rclSelf.PostponeTimeout(time);

			dcclite::Log::Debug("[{}::Device::OnPacket] pong", m_rclSelf.GetName());

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
			
			dcclite::Log::Warn("[{}::Device::OnPacket] task data, but not task running or task not found for id {}", m_rclSelf.GetName(), taskId);

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
		
		//dcclite::Log::Debug("[{}::Device::OnPacket] m_uOutgoingStatePacketAck {}", self.GetName(), m_uOutgoingStatePacketAck);

		bool stateRefresh = false;

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

			if (remoteDecoder->IsInputDecoder())
			{
				stateRefresh = true;
			}
		}

		if (stateRefresh)
		{
			SendStateDelta(true, time);
		}
	}

	void NetworkDevice::OnlineState::OnPingThink(const dcclite::Clock::TimePoint_t time)
	{
		m_clPingThinker.SetNext(time + PING_TIMEOUT);

		DevicePacket pkt{ dcclite::MsgTypes::MSG_PING, m_rclSelf.m_SessionToken, m_rclSelf.m_ConfigToken };
		m_rclSelf.m_clDccService.Device_SendPacket(m_rclSelf.m_RemoteAddress, pkt);

		//m_rclSelf.PostponeTimeout(time);

		dcclite::Log::Debug("[{}::Device::OnPacket] sending ping", m_rclSelf.GetName());
	}

	void NetworkDevice::OnlineState::Update(const dcclite::Clock::TimePoint_t time)
	{
		this->SendStateDelta(false, time);
	}

	void NetworkDevice::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Device::Serialize(stream);

		stream.AddBool("registered", m_fRegistered);
		stream.AddStringValue("configToken", dcclite::GuidToString(m_ConfigToken));
		stream.AddStringValue("sessionToken", dcclite::GuidToString(m_SessionToken));
		stream.AddStringValue("remoteAddress", m_RemoteAddress.GetIpString());
		stream.AddIntValue("connectionStatus", static_cast<int>(m_kStatus));

		m_clPinManager.Serialize(stream);
	}

	void NetworkDevice::Disconnect()
	{
		if (m_kStatus == Status::OFFLINE)
			return;

		DevicePacket pkt{ dcclite::MsgTypes::DISCONNECT, m_SessionToken, m_ConfigToken };

		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

		dcclite::Log::Warn("[{}::Device::Disconnect] Sent disconnect packet", this->GetName());

		this->GoOffline();
	}

	void NetworkDevice::GoOffline()
	{
		m_kStatus = Status::OFFLINE;

		m_clDccService.Device_UnregisterSession(*this, m_SessionToken);
		m_SessionToken = dcclite::Guid{};

		this->ClearState();
		this->AbortPendingTasks();

		dcclite::Log::Warn("[{}::Device::GoOffline] Is OFFLINE", this->GetName());
		m_clDccService.Device_NotifyStateChange(*this);
	}


	void NetworkDevice::AcceptConnection(const dcclite::Clock::TimePoint_t time, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken)
	{
		if (m_kStatus != Status::OFFLINE)
		{
			dcclite::Log::Error("[{}::Device::AcceptConnection] Already connected, cannot accept request from {}", this->GetName(), remoteAddress);

			return;
		}

		m_RemoteAddress = remoteAddress;
		m_SessionToken = dcclite::GuidCreate();

		m_kStatus = Status::CONNECTING;

		m_clDccService.Device_RegisterSession(*this, m_SessionToken);

		dcclite::Log::Info("[{}::Device::GoOnline] Is connecting", this->GetName());

		//this->RefreshTimeout(time);

		//Is device config expired?
		if (remoteConfigToken != m_ConfigToken)
		{
			dcclite::Log::Info("[{}::Device::AcceptConnection] Started configuring for token {}", this->GetName(), m_ConfigToken);

			this->GotoConfigState(time);

			dcclite::Log::Info("[{}::Device::AcceptConnection] config data sent", this->GetName());
		}
		else
		{
			//device config is fine...
			dcclite::Log::Info("[{}::Device::AcceptConnection] Accepted connection {} {}", this->GetName(), remoteAddress, m_ConfigToken);

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
			dcclite::Log::Warn("[{}::Device::CheckSession] Received packet from invalid config...", this->GetName());

			return false;
		}

		return true;
	}

	bool NetworkDevice::CheckSession(dcclite::NetworkAddress remoteAddress)
	{
		if (m_kStatus == Status::OFFLINE)
		{
			dcclite::Log::Error("[{}::Device::CheckSession] got packet from disconnected device", this->GetName());

			return false;
		}

		if (remoteAddress != m_RemoteAddress)
		{
			dcclite::Log::Warn("[{}::Device::CheckSession] Updating remote address, session valid", this->GetName());

			m_RemoteAddress = remoteAddress;
		}

		return true;
	}

	void NetworkDevice::PostponeTimeout(const dcclite::Clock::TimePoint_t time)
	{
		m_clTimeoutThinker.SetNext(time + TIMEOUT);
	}		

	void NetworkDevice::OnTimeoutThink(const dcclite::Clock::TimePoint_t time)
	{		
		dcclite::Log::Warn("[{}::Device::Update] timeout", this->GetName());

		this->GoOffline();
	}

	void NetworkDevice::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time, const dcclite::MsgTypes msgType, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteConfigToken)
	{
		if (!m_pclCurrentState)
		{
			dcclite::Log::Error("[{}::Device::OnPacket] Cannot process packet on Offline mode, packet: {}", this->GetName(), dcclite::MsgName(msgType));

			return;
		}

		//this->PostponeTimeout(time);

		m_pclCurrentState->OnPacket(packet, time, msgType, remoteAddress, remoteConfigToken);
	}

	void NetworkDevice::Update(const dcclite::Clock::TimePoint_t ticks)
	{
		if (m_kStatus == Status::OFFLINE)
			return;					

		m_pclCurrentState->Update(ticks);
	}

	void NetworkDevice::ClearState()
	{
		m_vState = NullState{};
		m_pclCurrentState = nullptr;
	}

	void NetworkDevice::GotoSyncState()
	{
		this->ClearState();

		m_vState.emplace<SyncState>(*this);
		m_pclCurrentState = std::get_if<SyncState>(&m_vState);

		dcclite::Log::Trace("[{}::Device::GotoSyncState] Entered", this->GetName());
	}

	void NetworkDevice::GotoOnlineState(const dcclite::Clock::TimePoint_t time)
	{
		this->ClearState();

		m_kStatus = Status::ONLINE;

		m_vState.emplace<OnlineState>(*this, time);
		m_pclCurrentState = std::get_if<OnlineState>(&m_vState);

		dcclite::Log::Trace("[{}::Device::GotoOnlineState] Entered", this->GetName());
		m_clDccService.Device_NotifyStateChange(*this);
	}

	void NetworkDevice::GotoConfigState(const dcclite::Clock::TimePoint_t time)
	{
		this->ClearState();

		m_vState.emplace<ConfigState>(*this, time);		
		m_pclCurrentState = std::get_if<ConfigState>(&m_vState);

		dcclite::Log::Trace("[{}::Device::GotoConfigState] Entered", this->GetName());
	}	

	bool NetworkDevice::IsConnectionStable() const noexcept
	{
		//
		//make sure we are connected and on online state (sync and config are not valids)
		return std::holds_alternative<NetworkDevice::OnlineState>(m_vState);		
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

	uint8_t NetworkDevice::TaskServices_FindDecoderIndex(const Decoder &decoder) const
	{
		assert(m_vecDecoders.size() < 255);

		for (size_t i = 0, len = m_vecDecoders.size(); i < len; ++i)
		{
			if (m_vecDecoders[i] == &decoder)
				return static_cast<uint8_t>(i);
		}

		throw std::out_of_range(fmt::format("[NetworkDevice::TaskServices_FindDecoderIndex] Decoder {} not found in device {}", decoder.GetName(), this->GetName()));
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
			throw std::runtime_error("[NetworkDevice::StartDownloadEEPromTask] Cannot start task without a connectd device");

		auto task = detail::StartDownloadEEPromTask(*this, ++g_u32TaskId, observer, resultsStorage);								

		m_lstTasks.push_back(task);		

		return task;
	}

	std::shared_ptr<NetworkTask> NetworkDevice::StartServoTurnoutProgrammerTask(NetworkTask::IObserver *observer, const std::string_view servoDecoderName)
	{		
		if (!this->IsConnectionStable())
			throw std::runtime_error("[NetworkDevice::StartServoTurnoutProgrammerTask] Cannot start task without a connectd device");

		auto obj = this->TryResolveChild(servoDecoderName);
		if (!obj)
			throw std::invalid_argument(fmt::format("[NetworkDevice::StartDownloadEEPromTask] Servo decoder {} not found", servoDecoderName));

		auto servoTurnout = dynamic_cast<ServoTurnoutDecoder *>(obj);
		if(!servoTurnout)
			throw std::invalid_argument(fmt::format("[NetworkDevice::StartDownloadEEPromTask] Servo decoder {} is not a ServoTurnoutDecoder", servoDecoderName));
		
		auto task = detail::StartServoTurnoutProgrammerTask(*this, ++g_u32TaskId, observer, *servoTurnout);
		
		m_lstTasks.push_back(task);		

		return task;
	}
}
