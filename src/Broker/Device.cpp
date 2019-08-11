// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Device.h"

#include <fstream>

#include <rapidjson/istreamwrapper.h>

#include "BitPack.h"
#include "Decoder.h"
#include "DccLiteService.h"
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

inline void PreparePacket(dcclite::Packet& packet, dcclite::MsgTypes msgType, const dcclite::Guid& sessionToken, const dcclite::Guid& configToken)
{
	dcclite::PacketBuilder builder{ packet, msgType, sessionToken, configToken };
}

Device::Device(std::string name, IDccDeviceServices &dccService, const rapidjson::Value &params, const Project &project) :
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(true)
{	
	const std::string deviceConfigFileName(std::string(this->GetName()) + ".decoders.json");

	const auto deviceConfigFilePath = project.GetFilePath(deviceConfigFileName);
	std::ifstream configFile(deviceConfigFilePath);
	if (!configFile)
	{
		dcclite::Log::Error("[Device::Device] cannot find {}", deviceConfigFilePath.string());

		return;
	}

	m_ConfigToken = project.GetFileToken(deviceConfigFileName);	

	dcclite::Log::Trace("[Device::Device] {} config token {}", this->GetName(), m_ConfigToken);
	dcclite::Log::Trace("[Device::Device] {} reading config {}", this->GetName(), deviceConfigFilePath.string());

	rapidjson::IStreamWrapper isw(configFile);
	rapidjson::Document decodersData;
	decodersData.ParseStream(isw);	

	if (!decodersData.IsArray())
		throw std::runtime_error("error: invalid config, expected decoders array inside Node");

	for (auto &element : decodersData.GetArray())
	{
		auto decoderName = element["name"].GetString();
		auto className = element["class"].GetString();
		Decoder::Address address{ element["address"] };

		auto &decoder = m_clDccService.Device_CreateDecoder(className, address, decoderName, element);

		m_vecDecoders.push_back(&decoder);

		this->AddChild(std::make_unique<dcclite::Shortcut>(std::string(decoder.GetName()), decoder));
	}

	dcclite::Log::Trace("[Device::Device] {} ready.", this->GetName());
}


Device::Device(std::string name, IDccDeviceServices &dccService):
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fPendingSync(true),
	m_fRegistered(false)
{
	//empty
}

void Device::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	FolderObject::Serialize(stream);

	stream.AddBool("registered", m_fRegistered);
	stream.AddStringValue("configToken", dcclite::GuidToString(m_ConfigToken));
	stream.AddStringValue("sessionToken", dcclite::GuidToString(m_SessionToken));
	stream.AddStringValue("remoteAddress", m_RemoteAddress.GetIpString());
	stream.AddBool("isOnline", this->IsOnline());
}

void Device::GoOnline(const dcclite::Address remoteAddress)
{	
	m_RemoteAddress = remoteAddress;
	m_SessionToken = dcclite::GuidCreate();
	m_clDccService.Device_RegisterSession(*this, m_SessionToken);

	m_uLastReceivedStatePacketId = 0;
	m_uOutgoingStatePacketId = 0;

	m_tLastStateSent.ClearAll();
	m_tLastStateSentTime -= STATE_TIMEOUT;

	m_eStatus = Status::ONLINE;		
	m_fPendingSync = true;
	m_SyncTimeout = {};

	dcclite::Log::Info("[{}::Device::GoOnline] Is online", this->GetName());
}

void Device::GoOffline()
{
	m_eStatus = Status::OFFLINE;	

	m_clDccService.Device_UnregisterSession(*this, m_SessionToken);
	m_SessionToken = dcclite::Guid{};
	m_upConfigState.reset();

	dcclite::Log::Warn("[{}::Device::GoOffline] Is OFFLINE", this->GetName());
}

void Device::SendDecoderConfigPacket(const size_t index) const
{
	dcclite::Packet pkt;

	PreparePacket(pkt, dcclite::MsgTypes::CONFIG_DEV, m_SessionToken, m_ConfigToken);
	pkt.Write8(static_cast<uint8_t>(index));

	m_vecDecoders[index]->WriteConfig(pkt);

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::SendConfigFinishedPacket() const
{
	dcclite::Packet pkt;

	PreparePacket(pkt, dcclite::MsgTypes::CONFIG_FINISHED, m_SessionToken, m_ConfigToken);
	pkt.Write8(static_cast<uint8_t>(m_vecDecoders.size()));

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::SendConfigStartPacket() const
{
	dcclite::Packet pkt;

	PreparePacket(pkt, dcclite::MsgTypes::CONFIG_START, m_SessionToken, m_ConfigToken);

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::AcceptConnection(const dcclite::Clock::TimePoint_t time, const dcclite::Address remoteAddress, const dcclite::Guid remoteSessionToken, const dcclite::Guid remoteConfigToken)
{
	if (m_eStatus == Status::ONLINE)
	{
		dcclite::Log::Error("[{}::Device::AcceptConnection] Already connected, cannot accept request from {}", this->GetName(), remoteAddress);

		return;
	}

	this->GoOnline(remoteAddress);	

	this->RefreshTimeout(time);

	if (remoteConfigToken != m_ConfigToken)
	{
		dcclite::Log::Info("[{}::Device::AcceptConnection] Started configuring for token {}", this->GetName(), m_ConfigToken);

		m_upConfigState = std::make_unique<ConfigInfo>();
		m_upConfigState->m_vecAcks.resize(m_vecDecoders.size());

		m_upConfigState->m_RetryTime = time + CONFIG_RETRY_TIME;		
		
		this->SendConfigStartPacket();

		for(size_t i = 0, sz = m_vecDecoders.size(); i < sz; ++i)		
		{
			this->SendDecoderConfigPacket(i);
		}		

		dcclite::Log::Info("[{}::Device::AcceptConnection] config data sent", this->GetName());		
	}
	else
	{
		dcclite::Log::Info("[{}::Device::AcceptConnection] Accepted connection {} {}", this->GetName(), remoteAddress, m_ConfigToken);

		dcclite::Packet pkt;
		PreparePacket(pkt, dcclite::MsgTypes::ACCEPTED, m_SessionToken, m_ConfigToken);
		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);		
	}	
}

void Device::OnPacket_Ping(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken)
{
	if (!this->CheckSessionConfig(remoteConfigToken, remoteAddress))
		return;

	dcclite::Packet pkt;
	PreparePacket(pkt, dcclite::MsgTypes::MSG_PONG, m_SessionToken, m_ConfigToken);
	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);	

	this->RefreshTimeout(time);

	dcclite::Log::Debug("[{}::Device::OnPacket_Ping] ping", this->GetName());
}

void Device::OnPacket_Sync(dcclite::Packet& packet, const dcclite::Clock::TimePoint_t time, const dcclite::Address remoteAddress, const dcclite::Guid remoteConfigToken)
{
	if (!this->CheckSessionConfig(remoteConfigToken, remoteAddress))
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
		m_vecDecoders[i]->SyncRemoteState(state);

		if (m_vecDecoders[i]->IsOutputDecoder())
		{
			auto* outputDecoder = static_cast<OutputDecoder*>(m_vecDecoders[i]);

			//if after a sync, the requested state changes, we toggle it, so they are synced
			//Hack?
			if (outputDecoder->GetPendingStateChange())
			{
				outputDecoder->ToggleState("OnPacket_Sync");
			}
		}
	}

	dcclite::Log::Info("[{}::Device::OnPacket_Sync] Sync OK", this->GetName());
	m_fPendingSync = false;
}

void Device::OnPacket_State(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken)
{
	//if out of sync, ignore states
	if (m_fPendingSync)
		return;

	if (!this->CheckSessionConfig(remoteConfigToken, remoteAddress))
		return;

	this->RefreshTimeout(time);

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

		m_vecDecoders[i]->SyncRemoteState(state);

		/**

		The remote device sends the state of any input (sensors)

		So if we received any of this, we send back to the client our current state so it can ACK our current state		
		*/

		if (m_vecDecoders[i]->IsInputDecoder())
		{
			stateRefresh = true;
		}
	}

	if (stateRefresh)
	{
		this->SendStateDelta(true, time);
	}
}

void Device::OnPacket_ConfigAck(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress)
{
	if (!this->CheckSession(remoteAddress))
		return;	

	auto seq = packet.Read<uint8_t>();

	if ((!m_upConfigState) || (seq >= m_upConfigState->m_vecAcks.size()))
	{
		dcclite::Log::Error("[{}::Device::OnPacket_ConfigAck] config out of sync, dropping connection", this->GetName());

		this->GoOffline();
	}

	//only increment seq count if m_vecAcks[seq] is not set yet, so we handle duplicate packets
	m_upConfigState->m_uSeqCount += m_upConfigState->m_vecAcks[seq] == false;
	m_upConfigState->m_vecAcks[seq] = true;	
	RefreshTimeout(time);	

	m_upConfigState->m_RetryTime = time + CONFIG_RETRY_TIME;

	dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config ACK {}", this->GetName(), seq);

	if (m_upConfigState->m_uSeqCount == m_upConfigState->m_vecAcks.size())
	{
		dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config Finished, configured {} decoders", this->GetName(), m_upConfigState->m_uSeqCount);

		this->SendConfigFinishedPacket();
	}
}

void Device::OnPacket_ConfigFinished(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress)
{
	if (!this->CheckSession(remoteAddress))
		return;

	m_upConfigState.reset();
	dcclite::Log::Info("[{}::Device::OnPacket_ConfigFinished] Config Finished, device is ready", this->GetName());

	RefreshTimeout(time);
}

bool Device::CheckSessionConfig(dcclite::Guid remoteConfigToken, dcclite::Address remoteAddress)
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



bool Device::CheckSession(dcclite::Address remoteAddress)
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

void Device::RefreshTimeout(dcclite::Clock::TimePoint_t time)
{
	m_Timeout = time + TIMEOUT;
}

bool Device::CheckTimeout(dcclite::Clock::TimePoint_t time)
{	
	if (time > m_Timeout)
	{
		dcclite::Log::Warn("[{}::Device::Update] timeout", this->GetName());

		this->GoOffline();

		return false;
	}

	return true;
}

void Device::SendStateDelta(const bool sendSensorsState, const dcclite::Clock::TimePoint_t time)
{
	dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> states;
	dcclite::BitPack<dcclite::MAX_DECODERS_STATES_PER_PACKET> changedStates;

	bool stateChanged = false;	

	const unsigned numDecoders = static_cast<unsigned>(std::min(m_vecDecoders.size(), size_t{ dcclite::MAX_DECODERS_STATES_PER_PACKET }));
	for (unsigned i = 0; i < numDecoders; ++i)
	{
		auto* decoder = m_vecDecoders[i];
		if (!decoder)
			continue;

		dcclite::DecoderStates state;

		if (decoder->IsOutputDecoder())
		{			
			auto* outputDecoder = static_cast<OutputDecoder*>(decoder);

			auto stateChange = outputDecoder->GetPendingStateChange();			
			if (!stateChange)
				continue;

			//dcclite::Log::Debug("SendStateDelta: change for {}", outputDecoder->GetName());

			state = stateChange.value();
		}
		else if ((sendSensorsState) && (decoder->IsInputDecoder()))
		{
			auto *inputDecoder = static_cast<SensorDecoder *>(decoder);

			state = inputDecoder->GetRemoteState();

			//dcclite::Log::Debug("SendStateDelta: change for {}", inputDecoder->GetName());
		}
		else
		{
			continue;
		}
		
		//mark on bit vector that this decoder has a change
		changedStates.SetBit(i);

		//Send down the decoder state
		states.SetBitValue(i, state == dcclite::DecoderStates::ACTIVE);

		stateChanged = true;
	}

	if (stateChanged)
	{
		if ((m_tLastStateSent == states) && ((time - m_tLastStateSentTime) < STATE_TIMEOUT))
			return;

		dcclite::Packet pkt;
		PreparePacket(pkt, dcclite::MsgTypes::STATE, m_SessionToken, m_ConfigToken);

		pkt.Write64(++m_uOutgoingStatePacketId);
		pkt.Write(changedStates);
		pkt.Write(states);

		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

		m_tLastStateSentTime = time;
		m_tLastStateSent = states;
	}
}

void Device::SendSyncRequest(const dcclite::Clock::TimePoint_t &ticks)
{
	assert(m_eStatus == Status::ONLINE);
	assert(!m_upConfigState);

	//too soon?
	if (m_SyncTimeout > ticks)
		return;

	dcclite::Log::Info("[{}::Device::SendSyncRequest] request sent", this->GetName());

	dcclite::Packet pkt;
	PreparePacket(pkt, dcclite::MsgTypes::SYNC, m_SessionToken, m_ConfigToken);

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);

	m_SyncTimeout = ticks + SYNC_TIMEOUT;
}

void Device::Update(const dcclite::Clock &clock)
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

	//Are we on config state and should retry sending config packets?
	if (m_upConfigState && (m_upConfigState->m_RetryTime < time))
	{
		//we havent received ack for some time, wake up the device (timeout keeps counting)
			
		//go thought all the decoders and check what does not have an ACK
		int pos = 0, packetCount = 0;
		for(const bool &acked : m_upConfigState->m_vecAcks)
		{
			if (!acked)
			{
				if (pos == 0)
				{
					//when config 0 is not received, this could also means that CONFIG_START was not received by tge remote, so we send it again
					this->SendConfigStartPacket();
				}

				dcclite::Log::Warn("[{}::Device::Update] retrying config for device {} at {}", this->GetName(), m_vecDecoders[pos]->GetName(), pos);
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
			dcclite::Log::Warn("[{}::Device::Update] retrying config finished for remote device", this->GetName());
			this->SendConfigFinishedPacket();
		}						
	}
	//No config state, so check for any requested state change and notify remote
	else if (!m_upConfigState)
	{		
		if (m_fPendingSync)
		{
			this->SendSyncRequest(time);
		}
		else
		{
			this->SendStateDelta(false, time);
		}		
	}	
}
