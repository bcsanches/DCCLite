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
        Stop                    = 0,
        TakeSiding              = 1,
        StopOrders              = 2,
        StopProceed             = 3,
        Restricted              = 4,
        Permissive              = 5,
        SlowAproach             = 6,
        Slow                    = 7,
        SlowMedium              = 8,
        SlowLimited             = 9,
        SlowClear               = 10,
        MediumAproach           = 11,
        MediumSlow              = 12,
        Medium                  = 13,
        MediumLimited           = 14,
        MediumClear             = 15,
        LimitedAproach          = 16,
        LimitedSlow             = 17,
        LimitedMedium           = 18,
        Limited                 = 19,
        LimitedClear            = 20,
        Aproach                 = 21,
        AdvanceAproach          = 22,
        AproachSlow             = 23,
        AdvanceAproachSlow      = 24,
        AproachMedium           = 25,
        AdvanceAproachMedium    = 26,
        AproachLimited          = 27,
        AdvanceAproachLimited   = 28,
        Clear                   = 29,
        CabSpeed                = 30,
        Dark                    = 31
	};

	void MakeSignalDecoderPacket(uint8_t dest[4], uint16_t outputAddr, SignalAspects aspect);
	std::tuple<uint16_t, SignalAspects> ExtractSignalDataFromPacket(const uint8_t packet[3]);    

    SignalAspects ConvertNameToAspect(const char *name);

    std::tuple<int16_t, uint16_t> ConvertAddressToNMRA(uint16_t address);
}
