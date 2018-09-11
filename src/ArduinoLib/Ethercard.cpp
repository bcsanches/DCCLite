#pragma once

#include "Ethercard.h"

#include "Serial.h"

EtherCard ether;

uint8_t EtherCard::myip[IP_LEN];   // my ip address
uint8_t EtherCard::dnsip[IP_LEN];  // dns server

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
	return 0;
}

bool EtherCard::dhcpSetup(const char *hname, bool fromRam)
{
	return true;
}

void EtherCard::udpServerListenOnPort(UdpServerCallback callback, uint16_t port)
{

}

void EtherCard::sendUdp(const char *data, uint8_t len, uint16_t sport, const uint8_t *dip, uint16_t dport)
{
	
}

uint16_t EtherCard::packetLoop(uint16_t plen)
{
	return 0;
}
