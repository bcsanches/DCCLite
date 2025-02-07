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

#include <cstdint>

#include "ArduinoLibDefs.h"

#define IP_LEN 4

class ARDUINO_API Ethernet
{
	public:
		//Not supported on emulator
		//static uint8_t buffer[];

		static uint16_t packetReceive();
};

//from https://github.com/njh/EtherCard/blob/master/src/EtherCard.h
/** This type definition defines the structure of a UDP server event handler callback function */
typedef void(*UdpServerCallback)(
	uint16_t dest_port,    ///< Port the packet was sent to
	uint8_t src_ip[IP_LEN],    ///< IP address of the sender
	unsigned int src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	unsigned int len);        ///< Length of the payload data

class ARDUINO_API EtherCard : public Ethernet
{
	public:
		static uint8_t netmask[IP_LEN];
		static uint8_t gwip[IP_LEN];
		static uint8_t myip[IP_LEN];    ///< IP address
		static uint8_t dnsip[IP_LEN];  ///< DNS server IP address

		static uint8_t begin(const uint16_t size, const uint8_t* macaddr, uint8_t csPin);

		static bool dhcpSetup(const char *hname = nullptr, bool fromRam = false);

		static void udpServerListenOnPort(UdpServerCallback callback, uint16_t port);
		static void udpServerPauseListenOnPort(uint16_t port);

		static void sendUdp(const char *data, uint8_t len, uint16_t sport, const uint8_t *dip, uint16_t dport);

		static uint16_t packetLoop(uint16_t plen);

		static void printIp(const uint8_t *buf);
		static void printIp(const char* msg, const uint8_t *buf);

		static void clientResolveIp(const uint8_t *ip);
		static bool clientWaitIp(const uint8_t *ip);
};

ARDUINO_API extern EtherCard ether;
