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

	extern void SendPacket(const uint8_t *data, uint8_t length, const uint8_t *destIp, uint16_t destPort);	

	extern void SetReceiverCallback(ReceiveCallback_t callback);

	extern void Update();

	extern void LogStatus();

	extern const char *GetNodeName() noexcept;
}

#endif
