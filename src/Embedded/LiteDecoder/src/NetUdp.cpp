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

#include "Ethercard.h"

#include "Console.h"
#include "Storage.h"
#include "Strings.h"

#include <avr/boot.h>

#define ARP_PATCH 1

#define MODULE_NAME F("NetUdp")

#define BUFFER_SIZE (256+96)

#ifndef BCS_ARDUINO_EMULATOR
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

	for(int i = 1;; ++i)
	{
		DCCLITE_LOG_MODULE_LN(F("ether begin try ") << i);

#ifdef ARDUINO_AVR_MEGA2560	
		if (ether.begin(BUFFER_SIZE, mac, 53) == 0)
#else
		if (ether.begin(BUFFER_SIZE, mac, 10) == 0)
#endif	
		{
			//Console::SendLogEx(MODULE_NAME, F("ether"), '.', F("begin"), ' ', FSTR_NOK);
			DCCLITE_LOG_MODULE_LN(F("ether begin") << FSTR_NOK);

			if(i == 5)
				return false;

			continue;
		}

		break;
	}

	//Console::SendLogEx(MODULE_NAME, F("net"), ' ', F("begin"), ' ', FSTR_OK);
	DCCLITE_LOG_MODULE_LN(F("ether begin ") << FSTR_OK);

#if 1
	for(int i = 0; !ether.dhcpSetup(g_szNodeName[0] ? g_szNodeName : nullptr, true); ++i)
	{
		//Console::SendLogEx(MODULE_NAME, F("dhcp"), ' ', FSTR_NOK," ", g_szNodeName);
		DCCLITE_LOG << MODULE_NAME << F("dhcp ") << FSTR_NOK << g_szNodeName << DCCLITE_ENDL;

		//return false;
		if(i == 10)
			return false;
	}		

	//Console::SendLogEx(MODULE_NAME, F("dhcp"), ' ', FSTR_OK);
	DCCLITE_LOG_MODULE_LN(F("dhcp ") << FSTR_OK);	
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


	ether.printIp("mask : ", ether.netmask);
	ether.printIp("GW IP: ", ether.gwip);
 	ether.printIp("DNS IP: ", ether.dnsip);
	ether.printIp("IP:  ", ether.myip);	

	//Console::SendLogEx(MODULE_NAME, FSTR_PORT, ':', ' ', g_iSrcPort);
	DCCLITE_LOG_MODULE_LN(FSTR_PORT << F(": ") << SRC_PORT);
	//ether.printIp("DNS: ", ether.dnsip);    

	//ether.parseIp(destip, "192.168.1.101");	

	//Console::SendLogEx(MODULE_NAME, FSTR_SETUP, ' ', FSTR_OK);
	DCCLITE_LOG_MODULE_LN(FSTR_OK);	

	ether.udpServerListenOnPort(callback, SRC_PORT);

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
	if((destIp[0] != 255) && (destIp[1] != 255) && (destIp[2] != 255) && (destIp[3] != 255) && ether.clientWaitIp(destIp))	
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_ARP, ' ', FSTR_NOK, ' ', Console::IpPrinter(destIp), FSTR_INVALID);
		Console::OutputStream stream;

		DCCLITE_LOG_MODULE_EX(stream) << FSTR_ARP << ' ' << FSTR_NOK << ' ';
		stream.IpNumber(destIp) << FSTR_INVALID << DCCLITE_ENDL;
		//DCCLITE_LOG << MODULE_NAME << FSTR_ARP << ' ' << FSTR_NOK << ' ' << Console::IpPrinter(destIp) << FSTR_INVALID << DCCLITE_ENDL;
	}

	ether.sendUdp(reinterpret_cast<const char *>(data), length, SRC_PORT, destIp, destPort );   
}

void NetUdp::Update()
{
	ether.packetLoop(ether.packetReceive());
}

//const char STATUS_STR[] = {"Name: %s, Mac: %X-%X-%X-%X-%X-%X, Port: %d"}; 

void NetUdp::LogStatus()
{
#if 0
	Console::SendLogEx(MODULE_NAME, FSTR_NAME, ':', ' ', 
		g_szNodeName, ' ',
		Console::Hex(g_u8Mac[0]), '-', Console::Hex(g_u8Mac[1]), '-', Console::Hex(g_u8Mac[2]), '-', Console::Hex(g_u8Mac[3]), '-', Console::Hex(g_u8Mac[4]), '-', Console::Hex(g_u8Mac[5]), ',', ' ',
		FSTR_PORT, ':', ' ', g_iSrcPort
	);

#else
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
#endif
}

const char *NetUdp::GetNodeName() noexcept
{
	return g_szNodeName;
}
