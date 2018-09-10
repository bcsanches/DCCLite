#ifndef NET_UDP_H
#define NET_UDP_H

#include <Arduino.h>

class EpromStream;

namespace NetUdp
{
	extern void LoadConfig(EpromStream &stream);
	extern void SaveConfig(EpromStream &stream);

	extern bool Configure(const char *nodeName, uint16_t port, const uint8_t *mac, const uint8_t *srvIp);

	extern bool Init();

	extern void SendPacket(const char *data, uint8_t length, const uint8_t *destIp, uint16_t destPort);

	extern void Update();

	extern void LogStatus();
}

#endif
