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

#include <stdint.h>

#include "NetUdp.h"

namespace Storage
{
	class EpromStream;
}

namespace dcclite
{
	class Guid;
	class Packet;
}

namespace Session
{
#ifdef ARDUINO_AVR_MEGA2560
	extern void LoadConfig(Storage::EpromStream &stream, bool oldConfig = false);
#else
	extern void LoadConfig(Storage::EpromStream &stream);
#endif

	extern void SaveConfig(Storage::EpromStream &stream);	

	extern bool Configure(uint16_t srvport);

	extern void Update(const unsigned long ticks, const bool stateChangeDetectedHint);

	extern void LogStatus();

	extern const dcclite::Guid &GetConfigToken();

	//This is to be only called by DecoderManager when loading its data
	extern void ReplaceConfigToken(const dcclite::Guid &configToken);

	extern NetUdp::ReceiveCallback_t GetReceiverCallback();

	extern void UpdateFreeRam(uint16_t freeRam);

	namespace detail
	{
		extern void InitTaskPacket(dcclite::Packet &packet, const uint32_t taskId);

		extern void SendTaskPacket(const dcclite::Packet &packet);
	}
}
