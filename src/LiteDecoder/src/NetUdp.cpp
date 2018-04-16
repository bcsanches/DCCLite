#include "NetUdp.h"

#include <Ethercard.h>

#include "Console.h"

#define MODULE_NAME "NetUdp"

static uint8_t g_u8Mac[] = { 0x00,0x00,0x00,0x00,0x00,0x00 };
byte Ethernet::buffer[512];

static int g_iSrcPort = 4551;

char textToSend[] = "test 123";

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


bool NetUdp::Init()
{
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
	ether.printIp("DNS: ", ether.dnsip);    

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
