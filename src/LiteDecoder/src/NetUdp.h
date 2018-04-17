#ifndef NET_UDP_H
#define NET_UDP_H

#include <Arduino.h>

class EpromStream;

namespace NetUdp
{
	extern void LoadConfig(EpromStream &stream);

	extern bool Init();

	extern void SendPacket(const char *data, uint8_t length, const uint8_t *destIp, uint16_t destPort);

	extern void Update();
}

#endif
