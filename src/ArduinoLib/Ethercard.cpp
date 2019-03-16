// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <array>
#include <vector>

#include "Ethercard.h"

#include "Serial.h"

#include <fmt/format.h>

#include <Socket.h>

EtherCard ether;

uint8_t EtherCard::myip[IP_LEN];   // my ip address
uint8_t EtherCard::dnsip[IP_LEN];  // dns server

static dcclite::Socket g_Socket;

//#define DROP

#ifdef DROP
static uint8_t g_uDropRate = 200;
#endif

typedef struct 
{
	UdpServerCallback callback;
	uint16_t port;	
}
UdpServerListener;

std::vector<UdpServerListener> g_vecListeners;

uint16_t Ethernet::packetReceive()
{
	return 0;
}

//direct from: https://github.com/njh/EtherCard/blob/master/src/webutil.cpp
void EtherCard::printIp(const char* msg, const uint8_t *buf) 
{
	Serial.print(msg);
	EtherCard::printIp(buf);
	Serial.println("");
}

//direct from: https://github.com/njh/EtherCard/blob/master/src/webutil.cpp
void EtherCard::printIp(const uint8_t *buf) 
{
	for (uint8_t i = 0; i < IP_LEN; ++i) 
	{
		Serial.print(buf[i]);
		if (i < 3)
			Serial.print('.');
	}
}

uint8_t EtherCard::begin(const uint16_t size, const uint8_t* macaddr, uint8_t csPin)
{
	return 1;
}

bool EtherCard::dhcpSetup(const char *hname, bool fromRam)
{
	return true;
}

void EtherCard::udpServerListenOnPort(UdpServerCallback callback, uint16_t port)
{
	if (g_Socket.IsOpen())
		throw std::logic_error("EtherCard::udpServerListenOnPort -> Only one port supported, sorry");

	if (!g_Socket.Open(port, dcclite::Socket::Type::DATAGRAM))
		throw std::runtime_error(fmt::format("EtherCard::udpServerListenOnPort: Cannot open datagram socket on port {}", port));

	UdpServerListener listener;

	listener.callback = callback;
	listener.port = port;

	g_vecListeners.push_back(listener);	
}

#ifdef DROP
static bool ShouldDrop()
{
	if ((rand() % 255) > g_uDropRate)
		return true;

	return false;
}
#endif

void EtherCard::sendUdp(const char *data, uint8_t len, uint16_t sport, const uint8_t *dip, uint16_t dport)
{
	dcclite::Address adr{ dip[0], dip[1], dip[2], dip[3], dport };

#ifdef DROP
	if (ShouldDrop())
		return;
#endif

	if (!g_Socket.Send(adr, data, len))
		throw std::logic_error(fmt::format("EtherCard::sendUdp: failed to send {} bytes", len));
}

uint16_t EtherCard::packetLoop(uint16_t plen)
{
	std::array<char, 2048> buffer;

	for(;;)
	{
		dcclite::Address sender;
		auto[status, size] = g_Socket.Receive(sender, buffer.data(), buffer.size());

		if (status != dcclite::Socket::Status::OK)
			break;

#ifdef DROPR
		if (ShouldDrop())
			continue;
#endif


		for (auto &listener : g_vecListeners)
		{
			std::uint8_t srcip[4];

			srcip[0] = sender.GetA();
			srcip[1] = sender.GetB();
			srcip[2] = sender.GetC();
			srcip[3] = sender.GetD();

			listener.callback(
				listener.port, 
				srcip, sender.GetPort(), 
				buffer.data(), 
				static_cast<std::uint16_t>(size)
			);
		}
	}	
	

	return 0;
}
