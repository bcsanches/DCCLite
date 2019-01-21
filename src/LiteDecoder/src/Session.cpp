#include "Session.h"

#include <stdint.h>

#include "Console.h"
#include "DecoderManager.h"
#include "EmbeddedLibDefs.h"
#include "NetUdp.h"
#include "Packet.h"
#include "Storage.h"

const char StorageModuleName[] PROGMEM = {"Session"} ;
#define MODULE_NAME Console::FlashStr(StorageModuleName)

enum class States
{
	OFFLINE,
	SEARCHING_SERVER,
	ONLINE,
	CONFIGURING
};

static dcclite::Guid g_SessionToken;
static dcclite::Guid g_ConfigToken;

static uint8_t g_u8ServerIp[] = { 0x00, 0x00, 0x00, 0x00 };
static uint16_t g_iSrvPort = 2424;

static unsigned long g_uTicks = 0;
static unsigned long g_uTimeoutTicks = 0;

static States g_eState = States::OFFLINE;

#define PING_TICKS 2500

#ifdef _DEBUG
#define TIMEOUT 10000
#else
#define TIMEOUT 10000
#endif


static void ReceiveCallback(
	uint8_t src_ip[4],    ///< IP address of the sender
	uint16_t src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	uint16_t len);


void Session::LoadConfig(EpromStream &stream)
{
	for (int i = 0; i < 4; ++i)
		stream.Get(g_u8ServerIp[i]);

	stream.Get(g_iSrvPort);

	Session::LogStatus();
}

void Session::SaveConfig(EpromStream &stream)
{
	for (int i = 0; i < 4; ++i)
		stream.Put(g_u8ServerIp[i]);

	stream.Put(g_iSrvPort);
}

bool Session::Configure(const uint8_t *srvIp, uint16_t srvport)
{
	memcpy(g_u8ServerIp, srvIp, sizeof(g_u8ServerIp));

	g_iSrvPort = srvport;

	return true;
}

bool Session::Init()
{
	NetUdp::SetReceiverCallback(ReceiveCallback);

	return true;
}



static void UpdatePingStatus(unsigned long currentTime)
{
	g_uTicks = currentTime + PING_TICKS;
	g_uTimeoutTicks = currentTime + TIMEOUT;
}

static bool Timeout(unsigned long currentTime)
{
	if (currentTime >= g_uTimeoutTicks)
	{
		//Server is dead?
		Console::SendLogEx(MODULE_NAME, "srv", ' ',  "timeout");

		g_eState = States::OFFLINE;

		return true;
	}

	return false;
}
	
static bool IsValidServer(uint8_t src_ip[4], uint16_t src_port)
{
	if (memcmp(src_ip, g_u8ServerIp, sizeof(g_u8ServerIp)) || (g_iSrvPort != src_port))
	{
		Console::SendLogEx(MODULE_NAME, "unknown", ' ', "ip");
		return false;
	}

	return true;
}

static void SendConfigPacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, uint8_t seq)
{
	packet.Reset();
	dcclite::PacketBuilder builder{ packet, msgType, g_SessionToken, g_ConfigToken };

	packet.Write8(seq);

	NetUdp::SendPacket(packet.GetData(), packet.GetSize(), g_u8ServerIp, g_iSrvPort);
}


static void GotoOnlineState()
{
	Console::SendLogEx(MODULE_NAME, "Online");

	g_eState = States::ONLINE;

	UpdatePingStatus(millis());
}

static void LogInvalidPacket(const Console::FlashStr &stateName, dcclite::MsgTypes type)
{
	Console::SendLogEx(MODULE_NAME, "invalid", ' ', "pkt", ' ', stateName, ' ', static_cast<int>(type));
}

static void OfflineTick()
{
	dcclite::Packet pkt;
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::HELLO, g_SessionToken, g_ConfigToken };

	builder.WriteStr(NetUdp::GetNodeName());

	uint8_t destip[4] = { 255, 255, 255, 255 };
	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), destip, g_iSrvPort);

	UpdatePingStatus(millis());	
	g_eState = States::SEARCHING_SERVER;
}

static void SearchingServerTick()
{
	if (!Timeout(millis()))
		return;

	OfflineTick();
}

const char OnSearchingServerPacketStateName[] PROGMEM = {"OnSearchingServerPacket"} ;
#define OnSearchingServerPacketStateNameStr Console::FlashStr(OnSearchingServerPacketStateName)

static void OnSearchingServerPacket(uint8_t src_ip[4], uint16_t src_port, dcclite::MsgTypes type, dcclite::Packet &packet)
{	
	if (type == dcclite::MsgTypes::ACCEPTED)
	{
		g_SessionToken = packet.ReadGuid();
		g_ConfigToken = packet.ReadGuid();

		GotoOnlineState();
	}	
	else if(type == dcclite::MsgTypes::CONFIG_START)
	{
		g_SessionToken = packet.ReadGuid();		

		g_eState = States::CONFIGURING;		
		g_uTicks = millis() + 1000;		
	}
	else
	{
		LogInvalidPacket(OnSearchingServerPacketStateNameStr, type);

		return;
	}	

	memcpy(g_u8ServerIp, src_ip, sizeof(g_u8ServerIp));
	g_iSrvPort = src_port;
}

static void OnlineTick()
{
	auto currentTime = millis();

	if (Timeout(currentTime))
		return;

	if (g_uTicks > currentTime)
		return;	

	dcclite::Packet pkt;
	
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::MSG_PING, g_SessionToken, g_ConfigToken };

	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), g_u8ServerIp, g_iSrvPort);
	g_uTicks = millis() + PING_TICKS;
}

const char OnOnlineStateName[] PROGMEM = {"Online"} ;
#define OnOnlineStateNameStr Console::FlashStr(OnOnlineStateName)

static void OnOnlinePacket(dcclite::MsgTypes type, dcclite::Packet &packet)
{		
	UpdatePingStatus(millis());	
	
	switch (type)
	{
		case dcclite::MsgTypes::MSG_PONG:
			//nothing to do, already done
			break;

		case dcclite::MsgTypes::CONFIG_FINISHED:		
			//this may happen when we send a CONFIG_FINISHED to server to ACK that we are configured but for some reason
			//server does not get it and so, it resends the CONFIG_FINISHED for us to ACK
			//we simple ignore and ack again to the server, yes
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);
			break;

		default:
			LogInvalidPacket(OnOnlineStateNameStr, type);
	}
}

static void ConfiguringTick()
{
	auto currentTime = millis();

	if (Timeout(currentTime))
		return;
}



static void HandleConfigPacket(dcclite::Packet &packet)
{	
	uint8_t seq = packet.Read<uint8_t>();
	
	auto device = DecoderManager::Create(seq, packet);

	SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_ACK, seq);
}

const char OnConfiguringPacketStateName[] PROGMEM = {"OnConfiguringPacket"} ;
#define OnConfiguringPacketStateNameStr Console::FlashStr(OnConfiguringPacketStateName)

void OnConfiguringPacket(dcclite::MsgTypes type, dcclite::Packet &packet)
{		
	switch (type)
	{
		case dcclite::MsgTypes::CONFIG_DEV:
			UpdatePingStatus(millis());
			HandleConfigPacket(packet);
			break;		

		default:
			LogInvalidPacket(OnConfiguringPacketStateNameStr, type);
			break;
	}
}


void Session::Update()
{
	switch (g_eState)
	{
		case States::OFFLINE:
			OfflineTick();
			break;

		case States::SEARCHING_SERVER:
			SearchingServerTick();
			break;

		case States::ONLINE:
			OnlineTick();
			break;

		case States::CONFIGURING:
			ConfiguringTick();
			break;
	}
}

static void ReceiveCallback(
	uint8_t src_ip[4],    ///< IP address of the sender
	uint16_t src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	uint16_t len)
{
	dcclite::Packet packet{ reinterpret_cast<const uint8_t*>(data), static_cast<uint8_t>(len) };

	if (packet.Read<uint32_t>() != dcclite::PACKET_ID)
	{
		Console::SendLogEx(MODULE_NAME, "invalid", ' ', "pkt", ' ', "id");

		return;
	}
	
	dcclite::MsgTypes type = packet.Read<dcclite::MsgTypes>();	

	if (g_eState == States::SEARCHING_SERVER)
	{
		//this state is special, we skip several checks
		OnSearchingServerPacket(src_ip, src_port, type, packet);

		return;
	}
	
	//does the packet comes from the known server?
	if (!IsValidServer(src_ip, src_port))
	{
		Console::SendLogEx(MODULE_NAME, "pkt", ' ', "from", ' ', "invalid", ' ', "srv", Console::IpPrinter(src_ip), ':', src_port);
		return;
	}

	dcclite::Guid token = packet.ReadGuid();
	if (token != g_SessionToken)
	{
		Console::SendLogEx(MODULE_NAME, "invalid", ' ', "session", ' ', "id");

		// g_eState = States::OFFLINE;
		return;
	}

	token = packet.ReadGuid();

	//if we are on config state, config_token is not set yet, so we ignore if for now and handle config packets
	if (g_eState == States::CONFIGURING)
	{		
		if (type == dcclite::MsgTypes::CONFIG_FINISHED)
		{			
			//let server know that we are in a good state
			SendConfigPacket(packet, dcclite::MsgTypes::CONFIG_FINISHED, 255);			

			//This is the place where we set the config token
			g_ConfigToken = token;

			GotoOnlineState();
		}
		else if(type == dcclite::MsgTypes::ACCEPTED)
		{
			GotoOnlineState();
		}
		else
		{
			OnConfiguringPacket(type, packet);
		}	

		return;
	}			
	
	//we have been already configured, so validate the config token
	if (token != g_ConfigToken)
	{
		Console::SendLogEx(MODULE_NAME, "invalid", ' ', "cfg", ' ', "id", ',', ' ', "going", ' ', "to", ' ', "offline");

		g_eState = States::OFFLINE;
		return;
	}

	OnOnlinePacket(type, packet);	
}

void Session::LogStatus()
{
	Console::SendLogEx(MODULE_NAME, "srv", ':', ' ', Console::IpPrinter(g_u8ServerIp), ':', g_iSrvPort);
}
