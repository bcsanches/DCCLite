#include "Session.h"

#include <stdint.h>

#include "Console.h"
#include "NetUdp.h"
#include "Packet.h"
#include "Storage.h"

#define MODULE_NAME "Session"

enum class States
{
	OFFLINE,
	SEARCHING_SERVER
};

static dcclite::Guid g_SessionToken;
static dcclite::Guid g_ConfigToken;

static uint8_t g_u8ServerIp[] = { 0x00, 0x00, 0x00, 0x00 };
static uint16_t g_iSrvPort = 2424;

static unsigned long g_uTicks = 0;

static States g_eState = States::OFFLINE;

static void ReceiveCallback(
	uint8_t src_ip[4],    ///< IP address of the sender
	uint16_t src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	uint16_t len)
{	
}

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
	NetUdp::RegisterCallback(ReceiveCallback);

	return true;
}

static void OnOffline()
{
	g_eState = States::SEARCHING_SERVER;

	dcclite::Packet pkt;
	dcclite::PacketBuilder builder{ pkt, dcclite::MsgTypes::HELLO, g_SessionToken, g_ConfigToken };

	builder.WriteStr(NetUdp::GetNodeName());

	uint8_t destip[4] = { 255, 255, 255, 255 };
	NetUdp::SendPacket(pkt.GetData(), pkt.GetSize(), destip, g_iSrvPort);

	g_uTicks = millis() + 1000;
	g_eState = States::SEARCHING_SERVER;
}

static void OnSearchingServer()
{
	if (g_uTicks > millis())
		return;

	OnOffline();
}

void Session::Update()
{
	switch (g_eState)
	{
		case States::OFFLINE:
			OnOffline();
			break;

		case States::SEARCHING_SERVER:
			OnSearchingServer();
			break;
	}
}

void Session::LogStatus()
{
	Console::SendLog(MODULE_NAME, "Srv: %d.%d.%d.%d:%d",		
		g_u8ServerIp[0], g_u8ServerIp[1], g_u8ServerIp[2], g_u8ServerIp[3],
		g_iSrvPort
	);
}
