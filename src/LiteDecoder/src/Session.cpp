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
#include "EmbeddedLibDefs.h"
#include "NetUdp.h"
#include "Packet.h"
#include "Storage.h"
#include "Strings.h"

const char StorageModuleName[] PROGMEM = {"Session"} ;
#define MODULE_NAME Console::FlashStr(StorageModuleName)

enum class ConnectionStates: uint8_t
{
	OFFLINE,
	SEARCHING_SERVER,	
	ARPDISCOVER,
	CONFIGURING,	
	ONLINE
};

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
static bool g_fForceStateRefresh = false;




//
//
// CONFIGURATION
//
//

void Session::LoadConfig(EpromStream &stream)
{
	for (int i = 0; i < 4; ++i)
		stream.Get(g_u8ServerIp[i]);

	stream.Get(g_uSrvPort);

	Session::LogStatus();
}

void Session::SaveConfig(EpromStream &stream)
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

bool Session::Init()
{
	//NetUdp::SetReceiverCallback(ReceiveCallback);

	return true;
}

//
//
// PING HANDLING
//
//

namespace PingManager
{
	static unsigned long g_uNextPingThink = 0;
	static unsigned long g_uTimeoutTicks = 0;

	static void Reset(unsigned long currentTime)
	{
		g_uNextPingThink = currentTime + Config::g_cfgPingTicks;
		g_uTimeoutTicks = currentTime + Config::g_cfgTimeoutTicks;
	}

	static bool CheckTimeout(unsigned long currentTime)
	{
		if (currentTime >= g_uTimeoutTicks)
		{
			//Server is dead?
			Console::SendLogEx(MODULE_NAME, "srv", ' ',  "timeout");

			g_eConnectionState = ConnectionStates::OFFLINE;

			return true;
		}

		return false;
	}

	static void Launch(unsigned long currentTime)
	{
		if (g_uNextPingThink <= currentTime)
		{		
			Console::SendLogEx(MODULE_NAME, "PING");
			Blinker::Play(Blinker::Animations::ERROR);

			dcclite::Packet pkt;

			dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::MSG_PING, g_SessionToken, g_ConfigToken };

			//Send three times... stupid network...
			NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);
			NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);
			NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);

			g_uNextPingThink = currentTime + Config::g_cfgPingTicks;
		}
	}
}



//
//
// MISC
//
//

static void LogInvalidPacket(const Console::FlashStr &stateName, dcclite::MsgTypes type)
{
	Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', "pkt", ' ', stateName, ' ', static_cast<int>(type));
}
	
static bool IsValidServer(uint8_t src_ip[4], uint16_t src_port)
{
	if (memcmp(src_ip, g_u8ServerIp, sizeof(g_u8ServerIp)) || (g_uSrvPort != src_port))
	{
		Console::SendLogEx(MODULE_NAME, FSTR_UNKNOWN, ' ', "ip");
		return false;
	}

	return true;
}

void Session::LogStatus()
{
	Console::SendLogEx(MODULE_NAME, "srv", ':', ' ', Console::IpPrinter(g_u8ServerIp), ':', g_uSrvPort);
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

static void OfflineTick(const unsigned long ticks)
{	
	Console::SendLogEx(MODULE_NAME, "Broadcast");

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

static void ConfiguringTick()
{
	//Only check for timeout here
	PingManager::CheckTimeout(millis());
}

static void SendConfigPacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, uint8_t seq)
{
	packet.Reset();
	dcclite::PacketBuilder builder{ packet, msgType, g_SessionToken, g_ConfigToken };

	packet.Write8(seq);

	NetUdp::SendPacket(packet.GetData(), packet.GetSize(), g_u8ServerIp, g_uSrvPort);
}

const char OnConfiguringPacketStateName[] PROGMEM = { "OnConfiguringPacket" };
#define OnConfiguringPacketStateNameStr Console::FlashStr(OnConfiguringPacketStateName)

static void HandleConfigPacket(dcclite::Packet &packet)
{
	uint8_t seq = packet.Read<uint8_t>();

	DecoderManager::Create(seq, packet);

	Console::SendLogEx(MODULE_NAME, OnConfiguringPacketStateNameStr, ' ', "Ack", ' ', seq);

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

const char OnSearchingServerPacketStateName[] PROGMEM = {"OnSearchingServerPacket"} ;
#define OnSearchingServerPacketStateNameStr Console::FlashStr(OnSearchingServerPacketStateName)

static void SendHelloPacket()
{
	dcclite::Packet pkt;
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::HELLO, g_SessionToken, g_ConfigToken };

	builder.WriteStr(NetUdp::GetNodeName());

	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);

	PingManager::Reset(millis());
}

static bool ArpTick(const unsigned long ticks)
{
	//Check if we got an ARP response
	if(NetUdp::IsIpCached(g_u8ServerIp))
	{
		//IP is in cache, go back to SEARCHING SERVER STATE
		Console::SendLogEx(MODULE_NAME, "ARP DONE");
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

static void GotoOnlineState(const unsigned long ticks)
{
	Console::SendLogEx(MODULE_NAME, "Online");

	g_eConnectionState = ConnectionStates::ONLINE;
	g_fForceStateRefresh = true;
	g_uLastReceivedDecodersStatePacket = 0;
	g_uDecodersStateSequence = 0;

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
			Console::SendLogEx(MODULE_NAME, "ARPING");
			g_eConnectionState = ConnectionStates::ARPDISCOVER;

			NetUdp::ResolveIp(src_ip);
		}
		else
		{
			//We already have the server IP on ARP cache, so let it knows we exists
			Console::SendLogEx(MODULE_NAME, "IP", ' ', "CACHED", ' ', Console::IpPrinter(g_u8ServerIp));

			SendHelloPacket();
		}						
	}	
	else if (type == dcclite::MsgTypes::ACCEPTED)
	{
		//If server accepted the hello, just go to SYNC STATE
		g_SessionToken = packet.ReadGuid();
		g_ConfigToken = packet.ReadGuid();

		GotoOnlineState(millis());
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

static void OnlineTick(const unsigned long ticks, const bool stateChangeDetectedHint)
{	
	if (PingManager::CheckTimeout(ticks))
		return;

	using namespace dcclite;	

	if (stateChangeDetectedHint || g_fForceStateRefresh || (g_uNextStateThink <= ticks))
	{		
		StatesBitPack_t states;
		StatesBitPack_t changedStates;		

#if 0
		if(g_fForceStateRefresh)
			Console::SendLogEx("SESSION", "State FORCE");
#endif

		if (g_fForceStateRefresh)		
			DecoderManager::WriteStates(changedStates, states);					
		else
			//If any delta, set flag so we dispatch the packet
			g_fForceStateRefresh = DecoderManager::ProduceStatesDelta(changedStates, states);

		if (g_fForceStateRefresh)
		{
			dcclite::Packet pkt;
			PacketBuilder builder{ pkt, MsgTypes::STATE, g_SessionToken, g_ConfigToken };

			pkt.Write64(++g_uDecodersStateSequence);
			pkt.Write(changedStates);
			pkt.Write(states);

			NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_uSrvPort);			

			g_fForceStateRefresh = false;
		}

		g_uNextStateThink = ticks + Config::g_cfgStateTicks;
	}
	
	PingManager::Launch(ticks);
}

static void OnStatePacket(dcclite::Packet &packet)
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

	Console::SendLogEx(MODULE_NAME, "state", (int)sequenceNumber);

	StatesBitPack_t states;
	StatesBitPack_t changedStates;

	packet.ReadBitPack(changedStates);
	packet.ReadBitPack(states);

	/*
	
	Updates all decoders, if any output decoder state is received, we will force a state refresh to server

	The refresh is required because the state here maybe does not change, but server has not ACK that we 
	have this state, so we send back a state pack to the server, so it knows what is going on here.

	*/	

	g_fForceStateRefresh = DecoderManager::ReceiveServerStates(changedStates, states);	
}

const char OnOnlineStateName[] PROGMEM = {"Online"} ;
#define OnOnlineStateNameStr Console::FlashStr(OnOnlineStateName)

static void OnOnlinePacket(dcclite::MsgTypes type, dcclite::Packet &packet)
{		
	PingManager::Reset(millis());	
	
	switch (type)
	{
		case dcclite::MsgTypes::MSG_PONG:
			Blinker::Play(Blinker::Animations::OK);
			Console::SendLogEx(MODULE_NAME, "PONG");
			//nothing to do, already done
			break;

		case dcclite::MsgTypes::CONFIG_FINISHED:		
			//this may happen when we send a CONFIG_FINISHED to server to ACK that we are configured but for some reason
			//server does not get it and so, it resends the CONFIG_FINISHED for us to ACK
			//we simple ignore and ack again to the server, yes
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);
			break;
		
		case dcclite::MsgTypes::STATE:
			//Console::SendLogEx(MODULE_NAME, "got state");
			OnStatePacket(packet);			
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
			ConfiguringTick();
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
		Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', "pkt", ' ', "id");

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
		Console::SendLogEx(MODULE_NAME, "pkt", ' ', "from", ' ', FSTR_INVALID, ' ', "srv", Console::IpPrinter(src_ip), ':', src_port);
		return;
	}

	dcclite::Guid token = packet.ReadGuid();
	if (token != g_SessionToken)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', FSTR_SESSION, ' ', "id");

		// g_eState = States::OFFLINE;
		return;
	}

	token = packet.ReadGuid();

	//if we are on config state, config_token is not set yet, so we ignore if for now and handle config packets
	if (g_eConnectionState == ConnectionStates::CONFIGURING)
	{		
		if (type == dcclite::MsgTypes::CONFIG_FINISHED)
		{			
			//let server know that we are in a good state
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);			

			//This is the place where we set the config token
			g_ConfigToken = token;

			Storage::SaveConfig();

			GotoOnlineState(millis());
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
		Console::SendLogEx(MODULE_NAME, FSTR_INVALID, ' ', "cfg", ' ', "id", ',', ' ', "to", ' ', "offline");

		g_eConnectionState = ConnectionStates::OFFLINE;
		return;
	}

	OnOnlinePacket(type, packet);	
}


NetUdp::ReceiveCallback_t Session::GetReceiverCallback()
{
	return ReceiveCallback;
}