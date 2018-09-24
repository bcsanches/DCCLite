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
	uint16_t src_port,    ///< Port the packet was sent from
	const char *data,   ///< UDP payload data
	uint16_t len);        ///< Length of the payload data

class ARDUINO_API EtherCard : public Ethernet
{
	public:
		static uint8_t myip[IP_LEN];    ///< IP address
		static uint8_t dnsip[IP_LEN];  ///< DNS server IP address

		static uint8_t begin(const uint16_t size, const uint8_t* macaddr, uint8_t csPin);

		static bool dhcpSetup(const char *hname = NULL, bool fromRam = false);

		static void udpServerListenOnPort(UdpServerCallback callback, uint16_t port);

		static void sendUdp(const char *data, uint8_t len, uint16_t sport, const uint8_t *dip, uint16_t dport);

		static uint16_t packetLoop(uint16_t plen);

		static void printIp(const uint8_t *buf);
		static void printIp(const char* msg, const uint8_t *buf);
};

ARDUINO_API extern EtherCard ether;
