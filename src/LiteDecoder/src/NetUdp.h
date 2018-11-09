#ifndef NET_UDP_H
#define NET_UDP_H

#include <Arduino.h>

class EpromStream;

namespace NetUdp
{
	typedef void(*ReceiveCallback_t)(		
		uint8_t src_ip[4],    ///< IP address of the sender
		uint16_t src_port,    ///< Port the packet was sent from
		const char *data,   ///< UDP payload data
		uint16_t len);        ///< Length of the payload data

	extern void LoadConfig(EpromStream &stream);
	extern void SaveConfig(EpromStream &stream);

	extern bool Configure(const char *nodeName, uint16_t port, const uint8_t *mac);

	extern bool Init();

	extern void SendPacket(const char *data, uint8_t length, const uint8_t *destIp, uint16_t destPort);

	extern void RegisterCallback(ReceiveCallback_t callback);

	extern void Update();

	extern void LogStatus();
}

#endif
