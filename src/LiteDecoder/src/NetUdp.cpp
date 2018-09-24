#include "NetUdp.h"

#include <Ethercard.h>

#include "Console.h"
#include "Storage.h"

#define MODULE_NAME "NetUdp"

static uint8_t g_u8Mac[] = { 0x00,0x00,0x00,0x00,0x00,0x00 };
static uint8_t g_u8ServerIp[] = {0x00, 0x00, 0x00, 0x00};

#ifndef BCS_ARDUINO_EMULATOR
uint8_t Ethernet::buffer[512];
#endif

static uint16_t g_iSrcPort = 4551;

#define MAX_NODE_NAME 16
static char g_szNodeName[MAX_NODE_NAME + 1];

char textToSend[] = "test 123";

enum States
{
	DISCONNECTED,
	CONNECTING,
	LOOKING_UP
};

static void UdpCallback(uint16_t dest_port,    	///< Port the packet was sent to
    uint8_t src_ip[IP_LEN],    					///< IP address of the sender
    uint16_t src_port,    						///< Port the packet was sent from
    const char *data,   						///< UDP payload data
    uint16_t len)
{
    Serial.println("Got udp packet");
    ether.printIp("SRV: ", src_ip);
    Serial.println(data);
}

//<nodeName> <mac> <port> <srvipv4>		

void NetUdp::LoadConfig(EpromStream &stream)
{
	for(int i = 0;i < MAX_NODE_NAME; ++i)
		stream.Get(g_szNodeName[i]);

	g_szNodeName[MAX_NODE_NAME] = 0;

	for(int i = 0;i < 6; ++i)
	{
		stream.Get(g_u8Mac[i]);
	}

	stream.Get(g_iSrcPort);

	for(int i = 0;i < 4; ++i)
		stream.Get(g_u8ServerIp[i]);

	NetUdp::LogStatus();
}

void NetUdp::SaveConfig(EpromStream &stream)
{
	for(int i = 0;i < MAX_NODE_NAME; ++i)
		stream.Put(g_szNodeName[i]);

	for(int i = 0;i < 6; ++i)
		stream.Put(g_u8Mac[i]);

	stream.Put(g_iSrcPort);

	for(int i = 0;i < 4; ++i)
		stream.Put(g_u8ServerIp[i]);
}

bool NetUdp::Configure(const char *nodeName, uint16_t port, const uint8_t *mac, const uint8_t *srvIp)
{
	strncpy(g_szNodeName, nodeName, sizeof(g_szNodeName));
	g_szNodeName[MAX_NODE_NAME] = 0;

	g_iSrcPort = port;

	memcpy(g_u8Mac, mac, sizeof(g_u8Mac));
	memcpy(g_u8ServerIp, srvIp, sizeof(g_u8ServerIp));

	return true;
}

bool NetUdp::Init()
{
	{	
		bool validMac = false;
		for(int i = 0;i < 6; ++i)
		{
			if(g_u8Mac[i])
			{
				validMac = true;
				break;
			}						
		}

		if(!validMac)
		{
			Console::SendLog(MODULE_NAME, "mac not set");

			return false;			
		}
	}

	if (ether.begin(512, g_u8Mac, 53) == 0) 
	{
		Console::SendLog(MODULE_NAME, "ether.begin failed");

		return false;
	}

	Console::SendLog(MODULE_NAME, "ether.begin ok");	

#if 1
	if (!ether.dhcpSetup())
	{
		Console::SendLog(MODULE_NAME, "DHCP failed");

		return false;
	}		

	Console::SendLog(MODULE_NAME, "ether.dhcpSetup ok");
#else

	if(!ether.staticSetup(ip, gw, dns))
	{
		Serial.println("static setup failed");
	}  
	Serial.println("static setup ok");
#endif

	ether.printIp("IP:  ", ether.myip);	
	Console::SendLog(MODULE_NAME, "port: %d", g_iSrcPort);
	//ether.printIp("DNS: ", ether.dnsip);    

	//ether.parseIp(destip, "192.168.1.101");	

	Console::SendLog(MODULE_NAME, "setup OK");

	ether.udpServerListenOnPort(UdpCallback, g_iSrcPort);

	return true;
}

void NetUdp::SendPacket(const char *data, uint8_t length, const uint8_t *destIp, uint16_t destPort)
{
	ether.sendUdp(data, length, g_iSrcPort, destIp, destPort );   
}

void NetUdp::Update()
{
	ether.packetLoop(ether.packetReceive());
}

void NetUdp::LogStatus()
{
	Console::SendLog(MODULE_NAME, "Name: %s, Mac: %X-%X-%X-%X-%X-%X, Port: %d, Srv: %d.%d.%d.%d", 
		g_szNodeName, 
		g_u8Mac[0], g_u8Mac[1], g_u8Mac[2], g_u8Mac[3], g_u8Mac[4], g_u8Mac[5], 
		g_iSrcPort,
		g_u8ServerIp[0], g_u8ServerIp[1], g_u8ServerIp[2], g_u8ServerIp[3] 
	);
}
