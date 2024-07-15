// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef NET_UDP_H
#define NET_UDP_H

#include <Arduino.h>

namespace Storage
{
	class EpromStream;
}

namespace NetUdp
{
	constexpr uint8_t MAX_NODE_NAME = 16;

	typedef void(*ReceiveCallback_t)(
		uint16_t dest_port,    ///< Port the packet was sent to
		uint8_t src_ip[4],    ///< IP address of the sender
		unsigned int src_port,    ///< Port the packet was sent from
		const char *data,   ///< UDP payload data
		unsigned int len);

#ifdef ARDUINO_AVR_MEGA2560
	extern void LoadConfig(Storage::EpromStream &stream, bool oldConfig = false);
#else	
	extern void LoadConfig(Storage::EpromStream &stream);
#endif

	extern void SaveConfig(Storage::EpromStream &stream);

	extern void Configure(const char *nodeName);

	extern bool Init(ReceiveCallback_t callback);

	extern void ResolveIp(const uint8_t *ip);
	extern bool IsIpCached(const uint8_t *ip);

	extern void SendPacket(const uint8_t *data, uint8_t length, const uint8_t *destIp, uint16_t destPort);	

	//extern void SetReceiverCallback(ReceiveCallback_t callback);

	extern void Update();

	extern void LogStatus();

	extern const char *GetNodeName() noexcept;
}

#endif
