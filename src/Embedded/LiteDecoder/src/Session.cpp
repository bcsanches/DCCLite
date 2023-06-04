// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Session.h"

#include <stdint.h>

#include "Blinker.h"
#include "Console.h"
#include "Config.h"
#include "Decoder.h"
#include "DecoderManager.h"
#include "NetUdp.h"
#include "Packet.h"
#include "ServoProgrammer.h"
#include "SharedLibDefs.h"
#include "Storage.h"
#include "Strings.h"

//testar: https://github.com/ntruchsess/arduino_uip

#define MODULE_NAME F("Session")

enum class ConnectionStates: uint8_t
{
	OFFLINE,
	SEARCHING_SERVER,	
	ARPDISCOVER,
	CONFIGURING,	
	ONLINE
};

static void GotoOfflineState();

static dcclite::Guid g_SessionToken;
static dcclite::Guid g_ConfigToken;

static uint8_t g_u8ServerIp[] = { 0x00, 0x00, 0x00, 0x00 };
static uint16_t g_uSrvPort = 2424;

static unsigned long g_uNextStateThink = 0;

/***
If my numbers are correct, we can send 1000 packets per second (each ms) and it will take us
5 millions centuries to overlap those counters. We probably will have a power outage before that ...
*/
static uint64_t g_uLastReceivedDecodersStatePacket = 0;
static uint64_t g_uDecodersStateSequence = 0;

static ConnectionStates g_eConnectionState = ConnectionStates::OFFLINE;
static bool g_fRefreshServerAboutOutputDecoders = false;

static uint16_t g_uRemoteRamValue = UINT16_MAX;
static uint16_t g_uFreeRam = UINT16_MAX;


//
//
// CONFIGURATION
//
//

void Session::LoadConfig(Storage::EpromStream &stream)
{
	for (int i = 0; i < 4; ++i)
		stream.Get(g_u8ServerIp[i]);

	stream.Get(g_uSrvPort);

	Session::LogStatus();
}

void Session::SaveConfig(Storage::EpromStream &stream)
{
	for (int i = 0; i < 4; ++i)
		stream.Put(g_u8ServerIp[i]);

	stream.Put(g_uSrvPort);
}

bool Session::Configure(const uint8_t *srvIp, uint16_t srvport)
{
	DecoderManager::DestroyAll();

	memcpy(g_u8ServerIp, srvIp, sizeof(g_u8ServerIp));

	g_uSrvPort = srvport;

	return true;
}

//
//
// PING HANDLING
//
//

namespace PingManager
{	
	static unsigned long g_uTimeoutTicks = 0;	

	static void Reset(unsigned long currentTime)
	{		
		g_uTimeoutTicks = currentTime + Config::g_cfgTimeoutTicks;
	}

	static bool CheckTimeout(unsigned long currentTime)
	{
		if (currentTime >= g_uTimeoutTicks)
		{
			//Server is dead?
			//Console::SendLogEx(MODULE_NAME, F("server"), ' ',  FSTR_TIMEOUT);
			DCCLITE_LOG_MODULE_LN(F("server") << ' ' << FSTR_TIMEOUT);

			GotoOfflineState();			

			return true;
		}

		return false;
	}
}


//
//
// MISC
//
//

static void LogInvalidPacket(const FlashStringHelper_t *fstr, dcclite::MsgTypes type)
{
	//Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', F("pkt"), ' ', fstr, ' ', static_cast<int>(type));
	DCCLITE_LOG_MODULE_LN(FSTR_INVALID << F("pkt ") << fstr << ' ' << static_cast<int>(type));
}
	
static bool IsValidServer(uint8_t src_ip[4], uint16_t src_port)
{
	if (memcmp(src_ip, g_u8ServerIp, sizeof(g_u8ServerIp)) || (g_uSrvPort != src_port))
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_UNKNOWN, ' ', F("ip"));
		DCCLITE_LOG_MODULE_LN(FSTR_UNKNOWN << F("ip "));
		return false;
	}

	return true;
}

void Session::LogStatus()
{
	//Console::SendLogEx(MODULE_NAME, "srv", ':', ' ', Console::IpPrinter(g_u8ServerIp), ':', g_uSrvPort);

	Console::OutputStream stream;

	stream << '[' << MODULE_NAME << ']' << F(" srv: ");
	stream.IpNumber(g_u8ServerIp) << ':' << g_uSrvPort << DCCLITE_ENDL;	
}

const dcclite::Guid &Session::GetConfigToken()
{
	return g_ConfigToken;
}

void Session::ReplaceConfigToken(const dcclite::Guid &configToken)
{
	g_ConfigToken = configToken;
}

//
//
// OFFLINE STATE
//
//

static void GotoOfflineState()
{
	ServoProgrammer::Stop();

	g_eConnectionState = ConnectionStates::OFFLINE;
}

static void OfflineTick(const unsigned long ticks)
{	
	//Console::SendLogEx(MODULE_NAME, FSTR_BROADCAST, ':', g_uSrvPort);
	DCCLITE_LOG_MODULE_LN(FSTR_BROADCAST << ':' << g_uSrvPort);

	//
	//Just send a broadcast DISCOVERY packet
	dcclite::Packet pkt;
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::DISCOVERY, g_SessionToken, g_ConfigToken };

	builder.WriteStr(NetUdp::GetNodeName());

	uint8_t destip[4] = { 255, 255, 255, 255 };
	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), destip, g_uSrvPort);

	//
	//reset PING 
	PingManager::Reset(ticks);
	
	g_eConnectionState = ConnectionStates::SEARCHING_SERVER;
}

//
//
// CONFIGURING STATE
//
//

static void ConfiguringTick(const unsigned long ticks)
{
	//Only check for timeout here
	PingManager::CheckTimeout(ticks);
}

static void SendConfigPacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, uint8_t seq)
{
	packet.Reset();
	dcclite::PacketBuilder builder{ packet, msgType, g_SessionToken, g_ConfigToken };

	packet.Write8(seq);

	NetUdp::SendPacket(packet.GetData(), packet.GetSize(), g_u8ServerIp, g_uSrvPort);
}

#define OnConfiguringPacketStateNameStr F("OnConfiguringPacket")

static void HandleConfigPacket(dcclite::Packet &packet)
{
	uint8_t seq = packet.Read<uint8_t>();

	DecoderManager::Create(seq, packet);

	//Console::SendLogEx(MODULE_NAME, OnConfiguringPacketStateNameStr, ' ', F("Ack"), ' ', seq);
	DCCLITE_LOG_MODULE_LN(OnConfiguringPacketStateNameStr << F(" Ack ") << seq);

	SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_ACK, seq);
}

void OnConfiguringPacket(dcclite::MsgTypes type, dcclite::Packet &packet)
{
	switch (type)
	{
		case dcclite::MsgTypes::CONFIG_DEV:
			PingManager::Reset(millis());
			HandleConfigPacket(packet);
			break;

		default:
			LogInvalidPacket(OnConfiguringPacketStateNameStr, type);
			break;
	}
}

//
//
// SEARCHING SERVER AND ARP STATE
//
//


static void SearchingServerTick(const unsigned long ticks)
{
	if(!PingManager::CheckTimeout(ticks))	
		return;

	//
	//If had a timeout, so we go to offline state and start again
	OfflineTick(ticks);
}

#define OnSearchingServerPacketStateNameStr F("OnSearchingServerPacket")

static void SendHelloPacket()
{
	dcclite::Packet pkt;
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::HELLO, g_SessionToken, g_ConfigToken };
	
	builder.WriteStr(NetUdp::GetNodeName());

	pkt.Write16(dcclite::PROTOCOL_VERSION);

	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);

	PingManager::Reset(millis());
}

static bool ArpTick(const unsigned long ticks)
{
	//Check if we got an ARP response
	if(NetUdp::IsIpCached(g_u8ServerIp))
	{
		//IP is in cache, go back to SEARCHING SERVER STATE
		//Console::SendLogEx(MODULE_NAME, FSTR_ARP,  FSTR_OK);
		DCCLITE_LOG_MODULE_LN(FSTR_ARP << FSTR_OK);
		g_eConnectionState = ConnectionStates::SEARCHING_SERVER;

		//Send an Hello to server
		SendHelloPacket();

		return true;
	}
	else
	{
		//Is arp timing out?
		PingManager::CheckTimeout(ticks);
	}

	return false;
}

#define OnOnlineStateNameStr F("Online")

static void GotoOnlineState(const unsigned long ticks, int callerLine)
{
	//Console::SendLogEx(MODULE_NAME, OnOnlineStateNameStr, ' ', __LINE__, ' ', callerLine);
	DCCLITE_LOG_MODULE_LN(OnOnlineStateNameStr << ' ' << __LINE__ << ' ' << callerLine);

	g_eConnectionState = ConnectionStates::ONLINE;	
	g_uLastReceivedDecodersStatePacket = 0;
	g_uDecodersStateSequence = 0;	

	g_uRemoteRamValue = UINT16_MAX;

	//
	//Force a full state refresh
	g_fRefreshServerAboutOutputDecoders = true;
	g_uNextStateThink = 0;

	PingManager::Reset(ticks);
}


static void OnSearchingServerPacket(uint8_t src_ip[4], uint16_t src_port, dcclite::MsgTypes type, dcclite::Packet &packet)
{	
	//
	//Server answered DISCOVERY PACKET
	if(type == dcclite::MsgTypes::DISCOVERY)
	{		
		//Grab server ip and port
		memcpy(g_u8ServerIp, src_ip, sizeof(g_u8ServerIp));
		g_uSrvPort = src_port;	

		//Is IP address on ARP CACHE?
		if(!NetUdp::IsIpCached(g_u8ServerIp))
		{
			//Not on ARP cache, so send an ARP request and go to ARPDISCOVER state
			//Console::SendLogEx(MODULE_NAME, FSTR_ARP, ' ', FSTR_INIT);
			DCCLITE_LOG_MODULE_LN(FSTR_ARP << ' ' << FSTR_INIT);

			g_eConnectionState = ConnectionStates::ARPDISCOVER;

			NetUdp::ResolveIp(src_ip);
		}
		else
		{
			//We already have the server IP on ARP cache, so let it knows we exists
			//Console::SendLogEx(MODULE_NAME, "IP", ' ', "CACHED", ' ', Console::IpPrinter(g_u8ServerIp));
			Console::OutputStream stream;

			DCCLITE_LOG_MODULE_EX(stream) <<  F("IP CACHED ");
			stream.IpNumber(g_u8ServerIp) << DCCLITE_ENDL;

			SendHelloPacket();
		}						
	}	
	else if (type == dcclite::MsgTypes::ACCEPTED)
	{
		//If server accepted the hello, just go to SYNC STATE
		g_SessionToken = packet.ReadGuid();
		g_ConfigToken = packet.ReadGuid();

		GotoOnlineState(millis(), __LINE__);
	}	
	else if(type == dcclite::MsgTypes::CONFIG_START)
	{
		//If are not configured like server, so we have to update everything
		
		//First grab session token
		g_SessionToken = packet.ReadGuid();
		
		//Remove all decoders
		DecoderManager::DestroyAll();

		g_eConnectionState = ConnectionStates::CONFIGURING;		
	}
	else
	{
		LogInvalidPacket(OnSearchingServerPacketStateNameStr, type);

		return;
	}		
}

//
//
// ONLINE STATE
//
//

/**
 * Here we have two purposes:
 * 	- First: detect if we must send a state packet to server
 * 	- Second: if we must send a state, decide what needs to be sent
 * 
 * 	There are three possible situations to force us to send a packet to server:
 * 		- We got a stateChangeDetectedHint, that means that:
 * 			- An output decoder changed a slow state (like a servo turnout)
 * 			- A sensor changed state (detected or stopped dectecting something)
 * 
 * 		- g_fRefreshServerAboutOutputDecoders is true
 * 			- This means that the server requested us to change a output decoder state, so we send the state back to server, so its knows
 * 				that we ack the request
 * 
 * 		- g_uNextStateThink expired: 
 * 			- this means that a previous sent state does not reached the server
 * 			- in such case, we only worry about sensors, because output decoders (like a servo),
 * 				the server will request it to change again (and this will force g_fRefreshServerAboutOutputDecoders to true, thus forcing a state to be sent)
 * 
 * 
 * 	We should only send sensors to the server when we are sure that their state was not ack by the server, otherwise, the server will keep
 * 	sending it state back to us (for a ack) and we will stay in a ping pong all the time
 * 
*/
static void OnlineTick(const unsigned long ticks, const bool stateChangeDetectedHint)
{	
	if (PingManager::CheckTimeout(ticks))
		return;	

	using namespace dcclite;	

	ServoProgrammer::Update(ticks);

	const bool stateTimeout = (g_uNextStateThink <= ticks);	
	const bool sendSensors = stateTimeout || stateChangeDetectedHint;

	//Is server freeRam value out of sync?
	if((stateTimeout) && (g_uRemoteRamValue != g_uFreeRam))
	{
		dcclite::Packet pkt;
		PacketBuilder builder{ pkt, MsgTypes::RAM_DATA, g_SessionToken, g_ConfigToken };

		pkt.Write16(g_uFreeRam);
		
		NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);
	}

	//if nothing to do?
	if((!sendSensors) && (!g_fRefreshServerAboutOutputDecoders))
		return;
			
	StatesBitPack_t states;
	StatesBitPack_t changedStates;						

#if 1
	if(stateChangeDetectedHint)
	{
		//Console::SendLogEx("SESSION", "State stateChangeDetectedHint");
		DCCLITE_LOG_MODULE_LN(F("State stateChangeDetectedHint"));
	}
#endif

	bool hasDataToSend = true;
	if(sendSensors && g_fRefreshServerAboutOutputDecoders)
	{
		//Send everything, outputs and sensors
		DecoderManager::WriteStates(changedStates, states);				
	}
	else if(sendSensors)
	{
		//send only sensors that have no ACK from server
		hasDataToSend = DecoderManager::ProduceStatesDelta(changedStates, states);
	}
	else //if g_fRefreshServerAboutOutputDecoders
	{
		//send only output decoders state
		DecoderManager::WriteOutputDecoderStates(changedStates, states);
	}

	//clear flag
	g_fRefreshServerAboutOutputDecoders = false;
				
	//Do we really have any data to send? (ProduceStatesDelta may have produced nothing)
	if (hasDataToSend)
	{
		dcclite::Packet pkt;
		PacketBuilder builder{ pkt, MsgTypes::STATE, g_SessionToken, g_ConfigToken };

		pkt.Write64(++g_uDecodersStateSequence);
		pkt.Write(changedStates);
		pkt.Write(states);			

		//finally, send it...
		NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);			
	}

	//wait a bit before sending sensors again...
	if(sendSensors)
		g_uNextStateThink = ticks + Config::g_cfgStateTicks;
}

static void OnStatePacket(dcclite::Packet &packet, const unsigned long time)
{
	using namespace dcclite;

	auto sequenceNumber = packet.Read<uint64_t>();

	//out of sequence?
	if (sequenceNumber <= g_uLastReceivedDecodersStatePacket)
	{
		//just drop it
		return;
	}

	g_uLastReceivedDecodersStatePacket = sequenceNumber;

	//Console::SendLogEx(MODULE_NAME, "state", (int)sequenceNumber);
	DCCLITE_LOG_MODULE_LN(F("state ") << (int)sequenceNumber);

	StatesBitPack_t states;
	StatesBitPack_t changedStates;

	packet.ReadBitPack(changedStates);
	packet.ReadBitPack(states);

	/*
	
	Updates all decoders, if any output decoder state is received, we will force a state refresh to server

	The refresh is required because the state here maybe does not change, but server has not ACK that we 
	have this state, so we send back a state pack to the server, so it knows what is going on here.

	*/	

	g_fRefreshServerAboutOutputDecoders = DecoderManager::ReceiveServerStates(changedStates, states, time) || g_fRefreshServerAboutOutputDecoders;
}

static void OnSyncPacket(dcclite::Packet &packet)
{
	using namespace dcclite;

	StatesBitPack_t states;
	StatesBitPack_t changedStates;

	DecoderManager::WriteStates(changedStates, states);					
			
	dcclite::Packet pkt;
	PacketBuilder builder{ pkt, MsgTypes::SYNC, g_SessionToken, g_ConfigToken };
			
	pkt.Write(changedStates);
	pkt.Write(states);

	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);
}

void Session::detail::InitTaskPacket(dcclite::Packet &packet, const uint32_t taskId)
{
	packet.Reset();
	dcclite::PacketBuilder builder{ packet, dcclite::MsgTypes::TASK_DATA, g_SessionToken, g_ConfigToken };
	
	packet.Write32(taskId);
}

void Session::detail::SendTaskPacket(const dcclite::Packet &packet)
{
	NetUdp::SendPacket(packet.GetData(), packet.GetSize(), g_u8ServerIp, g_uSrvPort);
}

static void DownloadEpromTaskHandler(dcclite::Packet &packet)
{
	const uint32_t taskId = packet.Read<uint32_t>();

	uint8_t sliceNumber = packet.ReadByte();

	const auto storageSize = Storage::Length();

	constexpr uint8_t SLICE_SIZE = 64;

	//
	//write out the data
	Session::detail::InitTaskPacket(packet, taskId);
	
	packet.Write8(sliceNumber);
	packet.Write8(static_cast<uint8_t>(storageSize / SLICE_SIZE));		//numSlices
	packet.Write8(SLICE_SIZE);						//sliceSize

	Storage::EpromStream stream(sliceNumber * SLICE_SIZE);

	for (int i = 0; i < SLICE_SIZE; ++i)
	{
		uint8_t data;

		stream.Get(data);
		packet.Write8(data);
	}

	NetUdp::SendPacket(packet.GetData(), packet.GetSize(), g_u8ServerIp, g_uSrvPort);
}

static void OnTaskRequestPacket(dcclite::Packet &packet)
{
	using namespace dcclite;

	NetworkTaskTypes taskType = static_cast<NetworkTaskTypes>(packet.ReadByte());

	switch (taskType)
	{
		case NetworkTaskTypes::TASK_DOWNLOAD_EEPROM:
			DownloadEpromTaskHandler(packet);
			break;

		case NetworkTaskTypes::TASK_SERVO_PROGRAMMER:
			ServoProgrammer::ParsePacket(packet);
			break;

		default:
			//Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', "task", ' ', static_cast<int>(taskType));
			DCCLITE_LOG_MODULE_LN(FSTR_INVALID << F(" task ") << static_cast<int>(taskType));
			break;
	}
}

static void OnOnlinePacket(dcclite::MsgTypes type, dcclite::Packet &packet)
{			
	const auto time = millis();
	PingManager::Reset(time);

	switch (type)
	{
		case dcclite::MsgTypes::MSG_PING:
			Blinker::Play(Blinker::Animations::OK);

			//Console::SendLogEx(MODULE_NAME, "PING");
			DCCLITE_LOG_MODULE_LN(F("PING"));

			{				
				dcclite::Packet pkt;
				dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::MSG_PONG, g_SessionToken, g_ConfigToken };

				NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);
			}
			break;

		case dcclite::MsgTypes::STATE:
			//Console::SendLogEx(MODULE_NAME, "got state");
			OnStatePacket(packet, time);
			break;		

		case dcclite::MsgTypes::CONFIG_FINISHED:		
			//this may happen when we send a CONFIG_FINISHED to server to ACK that we are configured but for some reason
			//server does not get it and so, it resends the CONFIG_FINISHED for us to ACK
			//we simple ignore and ack again to the server, yes
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);
			break;			

		case dcclite::MsgTypes::SYNC:
			OnSyncPacket(packet);
			break;

		case dcclite::MsgTypes::RAM_DATA:
			{
				//just update the value
				g_uRemoteRamValue = packet.Read<uint16_t>();
			}
			break;

		case dcclite::MsgTypes::TASK_REQUEST:
			OnTaskRequestPacket(packet);
			break;

		default:
			LogInvalidPacket(OnOnlineStateNameStr, type);
	}
}


//
//
// SESSION MAIN
//
//


void Session::Update(const unsigned long ticks, const bool stateChangeDetectedHint)
{
	switch (g_eConnectionState)
	{
		case ConnectionStates::OFFLINE:
			OfflineTick(ticks);
			break;

		case ConnectionStates::ARPDISCOVER:
			ArpTick(ticks);
			break;

		case ConnectionStates::SEARCHING_SERVER:
			SearchingServerTick(ticks);
			break;

		case ConnectionStates::ONLINE:
			OnlineTick(ticks, stateChangeDetectedHint);
			break;

		case ConnectionStates::CONFIGURING:
			ConfiguringTick(ticks);
			break;
	}
}

static void ReceiveCallback(
	uint16_t destPort,
	uint8_t src_ip[4],    ///< IP address of the sender
	unsigned int src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	unsigned int len)
{
	if(g_eConnectionState == ConnectionStates::ARPDISCOVER)
	{
		//On this state, nothing todo, ignore all udp packets
		return;
	}

	dcclite::Packet packet{ reinterpret_cast<const uint8_t*>(data), static_cast<uint8_t>(len) };

	if (packet.Read<uint32_t>() != dcclite::PACKET_ID)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', "pkt", ' ', "id");
		DCCLITE_LOG_MODULE_LN(FSTR_INVALID << F(" pkt id"));

		return;
	}
	
	dcclite::MsgTypes type = packet.Read<dcclite::MsgTypes>();	

	if (g_eConnectionState == ConnectionStates::SEARCHING_SERVER)
	{
		//this state is special, we skip several checks
		OnSearchingServerPacket(src_ip, src_port, type, packet);

		return;
	}
	
	//does the packet comes from the known server?
	if (!IsValidServer(src_ip, src_port))
	{
		//Console::SendLogEx(MODULE_NAME, "pkt", ' ', "from", ' ', FSTR_INVALID, ' ', "srv", Console::IpPrinter(src_ip), ':', src_port);

		Console::OutputStream stream;

		DCCLITE_LOG_MODULE_EX(stream) << F("pkt from ") << FSTR_INVALID << F(" srv");
		stream.IpNumber(src_ip) << ':' << src_port;		
		return;
	}

	dcclite::Guid token = packet.ReadGuid();
	if (token != g_SessionToken)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', FSTR_SESSION, ' ', "id");
		DCCLITE_LOG_MODULE_LN(FSTR_INVALID << ' ' << FSTR_SESSION << F(" id"));

		// g_eState = States::OFFLINE;
		return;
	}

	if(type == dcclite::MsgTypes::DISCONNECT)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_DISCONNECT, ' ', FSTR_OFFLINE);
		DCCLITE_LOG_MODULE_LN(FSTR_DISCONNECT << ' ' << FSTR_OFFLINE);

		GotoOfflineState();		
		return;
	}

	token = packet.ReadGuid();

	//if we are on config state, config_token is not set yet, so we ignore it for now and handle config packets
	if (g_eConnectionState == ConnectionStates::CONFIGURING)
	{		
		if (type == dcclite::MsgTypes::CONFIG_FINISHED)
		{			
			//let server know that we are in a good state
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);			

			//This is the place where we set the config token
			g_ConfigToken = token;

			Storage::SaveConfig();

			GotoOnlineState(millis(), __LINE__);
		}

		//Disabling this, looks like garbage now
#if 0
		else if(type == dcclite::MsgTypes::ACCEPTED)
		{
			GotoOnlineState();
		}
#endif
		else
		{
			OnConfiguringPacket(type, packet);
		}	

		return;
	}			
	
	//we have been already configured, so validate the config token
	if (token != g_ConfigToken)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', F("cfg"), ' ', F("id"), ',', ' ', F("to"), ' ', FSTR_OFFLINE);
		DCCLITE_LOG_MODULE_LN(FSTR_INVALID << F("cfg id, going to ") << FSTR_OFFLINE);

		GotoOfflineState();		
		return;
	}

	OnOnlinePacket(type, packet);	
}

void Session::UpdateFreeRam(uint16_t freeRam)
{
	g_uFreeRam = freeRam;
}

NetUdp::ReceiveCallback_t Session::GetReceiverCallback()
{
	return ReceiveCallback;
}
 