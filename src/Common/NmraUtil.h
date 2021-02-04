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

#include <string_view>
#include <tuple>

namespace dcclite
{
	enum class SignalAspects:uint8_t
	{
        STOP                    = 0,
        TAKE_SIDING             = 1,
        STOP_ORDERS             = 2,
        STOP_PROCEED            = 3,
        RESTRICTED              = 4,
        PERMISSIVE              = 5,
        SLOW_APROACH            = 6,
        SLOW                    = 7,
        SLOW_MEDIUM             = 8,
        SLOW_LIMITED            = 9,
        SLOW_CLEAR              = 10,
        MEDIUM_APROACH          = 11,
        MEDIUM_SLOW             = 12,
        MEDIUM                  = 13,
        MEDIUM_LIMITED          = 14,
        MEDIUM_CLEAR            = 15,
        LIMITED_APROACH         = 16,
        LIMITED_SLOW            = 17,
        LIMITED_MEDIUM          = 18,
        LIMITED                 = 19,
        LIMITED_CLEAR           = 20,
        APROACH                 = 21,
        ADVANCE_APROACH         = 22,
        APROACH_SLOW            = 23,
        ADVANCE_APROACH_SLOW    = 24,
        APROACH_MEDIUM          = 25,
        ADVANCE_APROACH_MEDIUM  = 26,
        APROACH_LIMITED         = 27,
        ADVANCE_APROACH_LIMITED = 28,
        CLEAR                   = 29,
        CAB_SPEED               = 30,
        DARK                    = 31
	};

	void MakeSignalDecoderPacket(uint8_t dest[4], uint16_t outputAddr, SignalAspects aspect);
	std::tuple<uint16_t, SignalAspects> ExtractSignalDataFromPacket(const uint8_t packet[3]);    
}
