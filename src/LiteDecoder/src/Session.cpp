#include "Session.h"

#include <stdint.h>

#include "Console.h"
#include "NetUdp.h"
#include "Storage.h"

#define MODULE_NAME "Session"

struct Guid
{
	union
	{
		uint8_t m_bId[8];
		uint64_t m_bBigId[2];
	};

	inline bool IsNull() const
	{
		return (m_bBigId[0] == 0) && (m_bBigId[1] == 0);
	}
};

enum class States
{
	DISCONNECTED,
	SEARCHING
};

static Guid g_SessionToken = { 0 };
static Guid g_ConfigToken = { 0 };

static uint8_t g_u8ServerIp[] = { 0x00, 0x00, 0x00, 0x00 };
static uint16_t g_iSrvPort = 2424;

static States g_eState = States::DISCONNECTED;

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

static void OnDisconnected()
{
	g_eState = States::SEARCHING;

	uint8_t destip[4] = { 255, 255, 255, 255 };
	NetUdp::SendPacket("hello", 5, destip, g_iSrvPort);
}

static void OnSearching()
{

}

void Session::Update()
{
	switch (g_eState)
	{
		case States::DISCONNECTED:
			OnDisconnected();
			break;

		case States::SEARCHING:
			OnSearching();
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
