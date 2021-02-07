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

#include "BitPack.h"
#include "IDccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "Project.h"
#include "SensorDecoder.h"

using namespace std::chrono_literals;

static auto constexpr TIMEOUT = 10s;
static auto constexpr CONFIG_RETRY_TIME = 300ms;
static auto constexpr STATE_TIMEOUT = 250ms;

static auto constexpr SYNC_TIMEOUT = 250ms;


class DevicePacket: public dcclite::Packet
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
	m_clPinManager(DecodeBoardName(params["class"].GetString())),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(true)	
{	
	this->Load();
}


NetworkDevice::NetworkDevice(std::string name, IDccLite_DeviceServices &dccService, const Project &project):
	Device(std::move(name), dccService, project),	
	m_clPinManager(ArduinoBoards::MEGA),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(false)
{
	//empty
}

NetworkDevice::~NetworkDevice()
{	
	this->Unload();
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
	NetworkDevice &self,
	dcclite::Packet &packet,
	const dcclite::Clock::TimePoint_t time,
	const dcclite::MsgTypes msgType,
	const dcclite::NetworkAddress remoteAddress,
	const dcclite::Guid remoteConfigToken
)
{
	dcclite::Log::Error("[{}::Device::{}::OnPacket] Unexpected msg type: {}", self.GetName(), this->GetName(), dcclite::MsgName(msgType));
}

//
//
// ConfigState
//
//

NetworkDevice::ConfigState::ConfigState(NetworkDevice &self, const dcclite::Clock::TimePoint_t time)
{
	this->m_vecAcks.resize(self.m_vecDecoders.size());
	this->m_RetryTime = time + CONFIG_RETRY_TIME;

	this->SendConfigStartPacket(self);

	for (size_t i = 0, sz = self.m_vecDecoders.size(); i < sz; ++i)
	{
		this->SendDecoderConfigPacket(self, i);
	}
}

void NetworkDevice::ConfigState::SendDecoderConfigPacket(const NetworkDevice &self, const size_t index) const
{
	DevicePacket pkt{ dcclite::MsgTypes::CONFIG_DEV, self.m_SessionToken, self.m_ConfigToken };
	pkt.Write8(static_cast<uint8_t>(index));

	self.m_vecDecoders[index]->WriteConfig(pkt);

	self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);
}

void NetworkDevice::ConfigState::SendConfigFinishedPacket(const NetworkDevice &self) const
{
	DevicePacket pkt{ dcclite::MsgTypes::CONFIG_FINISHED, self.m_SessionToken, self.m_ConfigToken };
	
	pkt.Write8(static_cast<uint8_t>(self.m_vecDecoders.size()));

	self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);
}

void NetworkDevice::ConfigState::SendConfigStartPacket(const NetworkDevice &self) const
{
	DevicePacket pkt { dcclite::MsgTypes::CONFIG_START, self.m_SessionToken, self.m_ConfigToken};
	
	self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);
}

void NetworkDevice::ConfigState::OnPacket_ConfigAck(
	NetworkDevice &self,
	dcclite::Packet &packet,
	const dcclite::Clock::TimePoint_t time,
	const dcclite::MsgTypes msgType,
	const dcclite::NetworkAddress remoteAddress,
	const dcclite::Guid remoteConfigToken)
{
	if (!self.CheckSession(remoteAddress))
		return;

	auto seq = packet.Read<uint8_t>();

	if (seq >= m_vecAcks.size())
	{
		dcclite::Log::Error("[{}::Device::OnPacket_ConfigAck] config out of sync, dropping connection", self.GetName());

		self.GoOffline();		

		return;
	}

	//only increment seq count if m_vecAcks[seq] is not set yet, so we handle duplicate packets
	m_uSeqCount += m_vecAcks[seq] == false;
	m_vecAcks[seq] = true;	

	m_RetryTime = time + CONFIG_RETRY_TIME;

	dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config ACK {} - {}", self.GetName(), seq, self.m_vecDecoders[seq]->GetName());

	if (m_uSeqCount == m_vecAcks.size())
	{
		dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config Finished, configured {} decoders", self.GetName(), m_uSeqCount);

		this->SendConfigFinishedPacket(self);
	}
}

void NetworkDevice::ConfigState::OnPacket_ConfigFinished(
	NetworkDevice &self,
	dcclite::Packet &packet,
	const dcclite::Clock::TimePoint_t time,
	const dcclite::MsgTypes msgType,
	const dcclite::NetworkAddress remoteAddress,
	const dcclite::Guid remoteConfigToken
)
{
	if (!self.CheckSession(remoteAddress))
		return;
	
	dcclite::Log::Info("[{}::Device::OnPacket_ConfigFinished] Config Finished, device is ready", self.GetName());	

	self.GotoSyncState();	
}

void NetworkDevice::ConfigState::OnPacket(
	NetworkDevice &self,
	dcclite::Packet &packet,
	const dcclite::Clock::TimePoint_t time,
	const dcclite::MsgTypes msgType,
	const dcclite::NetworkAddress remoteAddress,
	const dcclite::Guid remoteConfigToken)
{
	switch (msgType)
	{
		case dcclite::MsgTypes::CONFIG_ACK:
			this->OnPacket_ConfigAck(self, packet, time, msgType, remoteAddress, remoteConfigToken);
			break;

		case dcclite::MsgTypes::CONFIG_FINISHED:
			this->OnPacket_ConfigFinished(self, packet, time, msgType, remoteAddress, remoteConfigToken);
			break;

		default:
			State::OnPacket(self, packet, time, msgType, remoteAddress, remoteConfigToken);
			break;
	}	
}

void NetworkDevice::ConfigState::Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time)
{	
	//should retry sending config packets?	
	if(m_RetryTime > time)
		return ;
	
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
				this->SendConfigStartPacket(self);
			}

			dcclite::Log::Warn("[{}::Device::Update] retrying config for device {} at {}", self.GetName(), self.m_vecDecoders[pos]->GetName(), pos);
			this->SendDecoderConfigPacket(self, pos);

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
		dcclite::Log::Warn("[{}::Device::Update] retrying config finished for remote device", self.GetName());
		this->SendConfigFinishedPacket(self);
	}	
}

//
//
// SyncState
//
//

void NetworkDevice::SyncState::OnPacket(
	NetworkDevice &self,
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
		State::OnPacket(self, packet, time, msgType, remoteAddress, remoteConfigToken);

		return;
	}

	if (!self.CheckSessionConfig(remoteConfigToken, remoteAddress))
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
		self.m_vecDecoders[i]->SyncRemoteState(state);

		if (self.m_vecDecoders[i]->IsOutputDecoder())
		{
			auto *outputDecoder = static_cast<OutputDecoder *>(self.m_vecDecoders[i]);

			//if after a sync, the requested state changes, we toggle it, so they are synced
			//Hack?
			if (outputDecoder->GetPendingStateChange())
			{
				outputDecoder->ToggleState("OnPacket_Sync");
			}
		}
	}

	dcclite::Log::Info("[{}::Device::SyncState::OnPacket] Sync OK", self.GetName());

	self.GotoOnlineState();	
}

void NetworkDevice::SyncState::Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time)
{	
	assert(self.m_eStatus == Status::ONLINE);
	
	//too soon?
	if (m_SyncTimeout > time)
		return;

	dcclite::Log::Info("[{}::Device::SyncState::Update] request sent", self.GetName());

	DevicePacket pkt{ dcclite::MsgTypes::SYNC, self.m_SessionToken, self.m_ConfigToken};

	self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);

	m_SyncTimeout = time + SYNC_TIMEOUT;
}

//
//
// OnlineState
//
//

NetworkDevice::OnlineState::OnlineState()
{
	m_tLastStateSent.ClearAll();
	m_tLastStateSentTime = {};
}

void NetworkDevice::OnlineState::SendStateDelta(NetworkDevice &self, const bool sendSensorsState, const dcclite::Clock::TimePoint_t time)
{	
	dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> states;
	dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> changedStates;

	bool stateChanged = false;
	bool sensorDetected = false;

	const unsigned numDecoders = static_cast<unsigned>(std::min(self.m_vecDecoders.size(), size_t{ dcclite::MAX_DECODERS_STATES_PER_PACKET }));
	for (unsigned i = 0; i < numDecoders; ++i)
	{
		auto *decoder = self.m_vecDecoders[i];
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
			auto *decoder = self.m_vecDecoders[i];
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
		if ((m_tLastStateSent == states) && ((time - m_tLastStateSentTime) < STATE_TIMEOUT))
			return;

		DevicePacket pkt{dcclite::MsgTypes::STATE, self.m_SessionToken, self.m_ConfigToken};

		pkt.Write64(++m_uOutgoingStatePacketId);
		pkt.Write(changedStates);
		pkt.Write(states);

		self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);

		m_tLastStateSentTime = time;
		m_tLastStateSent = states;
	}
}


void NetworkDevice::OnlineState::OnPacket(
	NetworkDevice &self,
	dcclite::Packet &packet,
	const dcclite::Clock::TimePoint_t time,
	const dcclite::MsgTypes msgType,
	const dcclite::NetworkAddress remoteAddress,
	const dcclite::Guid remoteConfigToken
)
{
	if (!self.CheckSessionConfig(remoteConfigToken, remoteAddress))
		return;

	if (msgType == dcclite::MsgTypes::MSG_PING)
	{
		DevicePacket pkt{dcclite::MsgTypes::MSG_PONG, self.m_SessionToken, self.m_ConfigToken};
		self.m_clDccService.Device_SendPacket(self.m_RemoteAddress, pkt);

		self.RefreshTimeout(time);

		dcclite::Log::Debug("[{}::Device::OnPacket_Ping] ping", self.GetName());

		return;
	}

	if (msgType != dcclite::MsgTypes::STATE)
	{
		State::OnPacket(self, packet, time, msgType, remoteAddress, remoteConfigToken);

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

	bool stateRefresh = false;

	for (unsigned i = 0; i < changedStates.size(); ++i)
	{
		if (!changedStates[i])
			continue;

		auto state = states[i] ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;

		self.m_vecDecoders[i]->SyncRemoteState(state);

		/**

		The remote device sent the state of any input (sensors)

		So if we received any sensor state, we send back to the client our current state so it can ACK our current state
		*/

		if (self.m_vecDecoders[i]->IsInputDecoder())
		{
			stateRefresh = true;
		}
	}

	if (stateRefresh)
	{
		SendStateDelta(self, true, time);
	}
}

void NetworkDevice::OnlineState::Update(NetworkDevice &self, const dcclite::Clock::TimePoint_t time)
{	
	this->SendStateDelta(self, false, time);	
}

void NetworkDevice::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	FolderObject::Serialize(stream);

	stream.AddBool("registered", m_fRegistered);
	stream.AddStringValue("configToken", dcclite::GuidToString(m_ConfigToken));
	stream.AddStringValue("sessionToken", dcclite::GuidToString(m_SessionToken));
	stream.AddStringValue("remoteAddress", m_RemoteAddress.GetIpString());
	stream.AddBool("connected", this->IsOnline());

	m_clPinManager.Serialize(stream);
}

void NetworkDevice::Disconnect()
{
	if (m_eStatus != Status::ONLINE)
		return;	
	
	DevicePacket pkt{dcclite::MsgTypes::DISCONNECT, m_SessionToken, m_ConfigToken};
		
	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

	dcclite::Log::Warn("[{}::Device::Disconnect] Sent disconnect packet", this->GetName());	

	this->GoOffline();
}

void NetworkDevice::GoOffline()
{
	m_eStatus = Status::OFFLINE;	

	m_clDccService.Device_UnregisterSession(*this, m_SessionToken);
	m_SessionToken = dcclite::Guid{};	
	
	this->ClearState();

	dcclite::Log::Warn("[{}::Device::GoOffline] Is OFFLINE", this->GetName());
}


void NetworkDevice::AcceptConnection(const dcclite::Clock::TimePoint_t time, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken)
{
	if (m_eStatus == Status::ONLINE)
	{
		dcclite::Log::Error("[{}::Device::AcceptConnection] Already connected, cannot accept request from {}", this->GetName(), remoteAddress);

		return;
	}	

	m_RemoteAddress = remoteAddress;
	m_SessionToken = dcclite::GuidCreate();

	m_eStatus = Status::ONLINE;

	m_clDccService.Device_RegisterSession(*this, m_SessionToken);	

	dcclite::Log::Info("[{}::Device::GoOnline] Is connected", this->GetName());

	this->RefreshTimeout(time);

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
		DevicePacket pkt{dcclite::MsgTypes::ACCEPTED, m_SessionToken, m_ConfigToken};
		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);				

		//now sync it
		this->GotoSyncState();		
	}	
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
	if (m_eStatus != Status::ONLINE)
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

void NetworkDevice::RefreshTimeout(dcclite::Clock::TimePoint_t time)
{
	m_Timeout = time + TIMEOUT;
}

bool NetworkDevice::CheckTimeout(dcclite::Clock::TimePoint_t time)
{	
	if (time > m_Timeout)
	{
		dcclite::Log::Warn("[{}::Device::Update] timeout", this->GetName());

		this->GoOffline();

		return false;
	}

	return true;
}

void NetworkDevice::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time, const dcclite::MsgTypes msgType, const dcclite::NetworkAddress remoteAddress, const dcclite::Guid remoteConfigToken)
{
	if (!m_pclCurrentState)
	{
		dcclite::Log::Error("[{}::Device::OnPacket] Cannot process packet on Offline mode, packet: {}", this->GetName(), dcclite::MsgName(msgType));

		return;
	}

	this->RefreshTimeout(time);

	m_pclCurrentState->OnPacket(*this, packet, time, msgType, remoteAddress, remoteConfigToken);
}

void NetworkDevice::Update(const dcclite::Clock &clock)
{
	if (m_eStatus != Status::ONLINE)
		return;
	
	auto time = clock.Now();

	//Did remoted device timedout?
	if (!this->CheckTimeout(time))
	{
		//yes, ok get out and wait for it to come back
		return;
	}		

	m_pclCurrentState->Update(*this, time);
}

void NetworkDevice::ClearState()
{
	m_vState = NullState{};
	m_pclCurrentState = nullptr;
}

void NetworkDevice::GotoSyncState()
{
	this->ClearState();

	m_vState = SyncState{};
	m_pclCurrentState = std::get_if<SyncState>(&m_vState);
	
	dcclite::Log::Trace("[{}::Device::GotoSyncState] Entered", this->GetName());
}

void NetworkDevice::GotoOnlineState()
{
	this->ClearState();

	m_vState = OnlineState{};
	m_pclCurrentState = std::get_if<OnlineState>(&m_vState);	
	
	dcclite::Log::Trace("[{}::Device::GotoOnlineState] Entered", this->GetName());
}

void NetworkDevice::GotoConfigState(const dcclite::Clock::TimePoint_t time)
{
	this->ClearState();

	m_vState = ConfigState{*this, time};
	m_pclCurrentState = std::get_if<ConfigState>(&m_vState);	

	dcclite::Log::Trace("[{}::Device::GotoConfigState] Entered", this->GetName());
}
