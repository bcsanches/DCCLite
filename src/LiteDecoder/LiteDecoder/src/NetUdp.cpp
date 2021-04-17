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

#include "Ethercard.h"

#include "Console.h"
#include "Storage.h"
#include "Strings.h"

//#include <avr/pgmspace.h>

#define ARP_PATCH 1

const char StorageModuleName[] PROGMEM = {"NetUdp"} ;
#define MODULE_NAME Console::FlashStr(StorageModuleName)

static uint8_t g_u8Mac[] = { 0x00,0x00,0x00,0x00,0x00,0x00 };

#define BUFFER_SIZE (256+96)

#ifndef BCS_ARDUINO_EMULATOR
uint8_t Ethernet::buffer[BUFFER_SIZE];
#endif

static uint16_t g_iSrcPort = 4551;

#define MAX_NODE_NAME 16
static char g_szNodeName[MAX_NODE_NAME + 1];

enum States
{
	DISCONNECTED,
	CONNECTING,
	LOOKING_UP
};

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

	NetUdp::LogStatus();
}

void NetUdp::SaveConfig(EpromStream &stream)
{
	for(int i = 0;i < MAX_NODE_NAME; ++i)
		stream.Put(g_szNodeName[i]);

	for(int i = 0;i < 6; ++i)
		stream.Put(g_u8Mac[i]);

	stream.Put(g_iSrcPort);
}

bool NetUdp::Configure(const char *nodeName, uint16_t port, const uint8_t *mac)
{
	strncpy(g_szNodeName, nodeName, sizeof(g_szNodeName));
	g_szNodeName[MAX_NODE_NAME] = 0;

	g_iSrcPort = port;

	memcpy(g_u8Mac, mac, sizeof(g_u8Mac));	

	return true;
}

bool NetUdp::Init(ReceiveCallback_t callback)
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
			Console::SendLogEx(MODULE_NAME, "no", ' ', "mac");

			return false;			
		}
	}

#ifdef ARDUINO_AVR_MEGA2560	
	if (ether.begin(BUFFER_SIZE, g_u8Mac, 53) == 0)
#else
	if (ether.begin(BUFFER_SIZE, g_u8Mac, 10) == 0)
#endif
	{
		Console::SendLogEx(MODULE_NAME, "ether", '.', "begin", ' ', FSTR_NOK);

		return false;
	}

	Console::SendLogEx(MODULE_NAME, "net", ' ', "begin", ' ', FSTR_OK);

#if 1
	for(int i = 0; !ether.dhcpSetup(g_szNodeName, true); ++i )
	{
		Console::SendLogEx(MODULE_NAME, "dhcp", ' ', FSTR_NOK," ", g_szNodeName);

		//return false;
		if(i == 10)
			return false;
	}		

	Console::SendLogEx(MODULE_NAME, "dhcp", ' ', FSTR_OK);
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
	Console::SendLogEx(MODULE_NAME, FSTR_PORT, ':', ' ', g_iSrcPort);
	//ether.printIp("DNS: ", ether.dnsip);    

	//ether.parseIp(destip, "192.168.1.101");	

	Console::SendLogEx(MODULE_NAME, FSTR_SETUP, ' ', FSTR_OK);

	ether.udpServerListenOnPort(callback, g_iSrcPort);

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
		Console::SendLogEx(MODULE_NAME, FSTR_ARP, ' ', FSTR_NOK, ' ', Console::IpPrinter(destIp), FSTR_INVALID);
	}

	ether.sendUdp(reinterpret_cast<const char *>(data), length, g_iSrcPort, destIp, destPort );   
}

void NetUdp::Update()
{
	ether.packetLoop(ether.packetReceive());
}

//const char STATUS_STR[] = {"Name: %s, Mac: %X-%X-%X-%X-%X-%X, Port: %d"}; 

void NetUdp::LogStatus()
{
	Console::SendLogEx(MODULE_NAME, FSTR_NAME, ':', ' ', 
		g_szNodeName, ' ',
		Console::Hex(g_u8Mac[0]), '-', Console::Hex(g_u8Mac[1]), '-', Console::Hex(g_u8Mac[2]), '-', Console::Hex(g_u8Mac[3]), '-', Console::Hex(g_u8Mac[4]), '-', Console::Hex(g_u8Mac[5]), ',', ' ',
		FSTR_PORT, ':', ' ', g_iSrcPort
	);
}

const char *NetUdp::GetNodeName() noexcept
{
	return g_szNodeName;
}
