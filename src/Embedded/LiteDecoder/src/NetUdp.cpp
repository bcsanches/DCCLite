// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NetUdp.h"

#include <dcclite_shared/Misc.h>
#include <dcclite_shared/Packet.h>

#ifdef NET_W5500
#include <Ethernet.h>
#include <EthernetUdp.h>
#else
#include "Ethercard.h"
#endif

#include "Console.h"
#include "Storage.h"
#include "Strings.h"

#include <avr/boot.h>

#define BUFFER_SIZE (256+96)

#ifdef NET_W5500

static EthernetUDP g_Udp;
static NetUdp::ReceiveCallback_t g_pfnCallback = nullptr;

static uint8_t g_u8Buffer[BUFFER_SIZE];

#define IP_LEN 4

static void printIp (const __FlashStringHelper *ifsh, const IPAddress &ip) 
{
    Serial.print(ifsh);
    Serial.println(ip);    
}

#else

#define ARP_PATCH 1

#endif

#define MODULE_NAME F("NetUdp")

#if (!defined BCS_ARDUINO_EMULATOR) && (!defined NET_W5500)
uint8_t Ethernet::buffer[BUFFER_SIZE];
#endif

constexpr uint16_t SRC_PORT = 7203;

static char g_szNodeName[dcclite::MAX_NODE_NAME + 1] = { 0 };

enum States
{
	DISCONNECTED,
	CONNECTING,
	LOOKING_UP
};

//<nodeName> <mac> <port>

void NetUdp::LoadConfig(Storage::EpromStream &stream, bool oldConfig)
{
	for(int i = 0;i < dcclite::MAX_NODE_NAME; ++i)
		stream.Get(g_szNodeName[i]);

	g_szNodeName[dcclite::MAX_NODE_NAME] = 0;

	if(oldConfig)
	{
		uint8_t mac;
		for(int i = 0;i < 6; ++i)
		{
			stream.Get(mac);
		}

		uint16_t unusedSrcPort;
		stream.Get(unusedSrcPort);
	}	

	NetUdp::LogStatus();
}

void NetUdp::SaveConfig(Storage::EpromStream &stream)
{
	for(int i = 0;i < dcclite::MAX_NODE_NAME; ++i)
		stream.Put(g_szNodeName[i]);
}

void NetUdp::Configure(dcclite::StringView nodeName)
{
	auto size = dcclite::MyMin(sizeof(g_szNodeName), nodeName.GetSize());

	strncpy(g_szNodeName, nodeName.GetData(), size);
	g_szNodeName[size] = 0;
}

static void GenerateMac(uint8_t mac[6])
{
	for(int i = 0;i < 6; ++i)
	{
		// take the fist 3 and last 3 of the serial.
		// the first 5 of 8 are at 0x0E to 0x013
		// the last  3 of 8 are at 0x15 to 0x017
		mac[i] = boot_signature_byte_get(0x0E + i + (i>2? 4 : 0));
    }

    mac[0] &= 0xFE;
    mac[0] |= 0x02;
  }

bool NetUdp::Init(ReceiveCallback_t callback)
{	
	uint8_t mac[6];

	GenerateMac(mac);
	{
		auto stream = DCCLITE_LOG_MODULE_EX(Console::OutputStream{}) << F("mac ");

		for(int i = 0;i < 6; ++i)
			stream.HexNumber(mac[i]) << ':';

		stream << DCCLITE_ENDL;
	}	

#ifdef NET_W5500
	for(int i = 1;; ++i)
	{
		DCCLITE_LOG_MODULE_LN(F("ether begin try ") << i);

		#ifdef ARDUINO_AVR_MEGA2560	
			Ethernet.init(53); // CS pin
		#else
			Ethernet.init(10); // CS pin
		#endif	

		if (!Ethernet.begin(mac))
		{			
			DCCLITE_LOG_MODULE_LN(F("Ethernet begin") << FSTR_NOK);

			if(i == 5)
				return false;

			continue;
		}
		break;
	}

	DCCLITE_LOG_MODULE_LN(F("ether begin ") << FSTR_OK);
	DCCLITE_LOG_MODULE_LN(F("dhcp OK"));

	g_pfnCallback = callback;

	printIp(F("mask : "), Ethernet.subnetMask());
	printIp(F("GW IP: "), Ethernet.gatewayIP());
	printIp(F("DNS IP: "), Ethernet.dnsServerIP());
	printIp(F("IP:  "), Ethernet.localIP());

	DCCLITE_LOG_MODULE_LN(FSTR_PORT << F(": ") << SRC_PORT);	
	DCCLITE_LOG_MODULE_LN(FSTR_OK);	

	g_Udp.begin(SRC_PORT);

#else
	for(int i = 1;; ++i)
	{
		DCCLITE_LOG_MODULE_LN(F("ether begin try ") << i);

	#ifdef ARDUINO_AVR_MEGA2560	
		if (ether.begin(BUFFER_SIZE, mac, 53) == 0)
	#else
		if (ether.begin(BUFFER_SIZE, mac, 10) == 0)
	#endif	
		{			
			DCCLITE_LOG_MODULE_LN(F("ether begin") << FSTR_NOK);

			if(i == 5)
				return false;

			continue;
		}

		break;
	}

	DCCLITE_LOG_MODULE_LN(F("ether begin ") << FSTR_OK);

	#if 1
		for(int i = 0; !ether.dhcpSetup(g_szNodeName[0] ? g_szNodeName : nullptr, true); ++i)
		{		
			DCCLITE_LOG_MODULE_LN(F("dhcp NOK") << ' ' << g_szNodeName);
			
			if(i == 10)
				return false;
		}		
		
		DCCLITE_LOG_MODULE_LN(F("dhcp OK"));
	#else

		uint8_t ip[] = {192,168,0,180};
		uint8_t gw[] = {192,168,0,1};
		uint8_t dns[] = {0,0,0,0};
		uint8_t mask[] = {255, 255, 255, 0};
		if(!ether.staticSetup(ip, gw, dns, mask))
		{
			Serial.println("static setup failed");
		}  
		Serial.println("static setup ok");

	#endif

	ether.printIp(F("mask : "), ether.netmask);
	ether.printIp(F("GW IP: "), ether.gwip);
 	ether.printIp(F("DNS IP: "), ether.dnsip);
	ether.printIp(F("IP:  "), ether.myip);

	ether.udpServerListenOnPort(callback, SRC_PORT);

	DCCLITE_LOG_MODULE_LN(FSTR_PORT << F(": ") << SRC_PORT);	
	DCCLITE_LOG_MODULE_LN(FSTR_OK);	

#endif		

	return true;
}

void NetUdp::ResolveIp(const uint8_t *ip)
{
#if ARP_PATCH
	ether.clientResolveIp(ip);
#endif
}
	
bool NetUdp::IsIpCached(const uint8_t *ip)
{
#if ARP_PATCH
	return !ether.clientWaitIp(ip);
#else
	return true;
#endif
}

void NetUdp::SendPacket(const uint8_t *data, uint8_t length, const uint8_t *destIp, uint16_t destPort)
{		
#ifdef NET_W5500

	g_Udp.beginPacket(IPAddress(destIp[0], destIp[1], destIp[2], destIp[3]), destPort);
	g_Udp.write(data, length);
	g_Udp.endPacket();	

#else
	if((destIp[0] != 255) && (destIp[1] != 255) && (destIp[2] != 255) && (destIp[3] != 255) && ether.clientWaitIp(destIp))	
	{		
		Console::OutputStream stream;

		DCCLITE_LOG_MODULE_EX(stream) << FSTR_ARP << ' ' << FSTR_NOK << ' ';
		stream.IpNumber(destIp) << FSTR_INVALID << DCCLITE_ENDL;		
	}

	ether.sendUdp(reinterpret_cast<const char *>(data), length, SRC_PORT, destIp, destPort );   
#endif
}

void NetUdp::Update()
{
#ifdef NET_W5500
	Ethernet.maintain();

	auto packetSize = g_Udp.parsePacket();
	if(!packetSize)
		return;

	g_Udp.read(g_u8Buffer, sizeof(g_u8Buffer));

	uint8_t rawIp[IP_LEN];
	IPAddress remoteIp = g_Udp.remoteIP();
	rawIp[0] = remoteIp[0];
	rawIp[1] = remoteIp[1];
	rawIp[2] = remoteIp[2];
	rawIp[3] = remoteIp[3];

	g_pfnCallback(
		g_Udp.localPort(),
		rawIp,
		g_Udp.remotePort(),		
		g_u8Buffer,
		packetSize
	);

#else
	ether.packetLoop(ether.packetReceive());
#endif
}

//const char STATUS_STR[] = {"Name: %s, Mac: %X-%X-%X-%X-%X-%X, Port: %d"}; 

void NetUdp::LogStatus()
{
	Console::OutputStream output;

	DCCLITE_LOG_MODULE_EX(output) << FSTR_NAME << ": " << g_szNodeName << ' ';

	uint8_t mac[6];

	GenerateMac(mac);

	for(int i = 0;i < 6; ++i)
	{
		output.HexNumber(mac[i]);
		output << ' ';
	}

	output << ' ' << FSTR_PORT << F(": ") << SRC_PORT << DCCLITE_ENDL;
}

const char *NetUdp::GetNodeName() noexcept
{
	return g_szNodeName;
}
