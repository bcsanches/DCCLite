// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include "NmraUtil.h"

using namespace dcclite;

TEST(NmraUtil, dccLiteExtractSignalAddressFromPacket)
{					

	for (uint16_t i = 1, aspect = 0; i < 2048; ++i, ++aspect)
	{
		uint8_t packet[4];

		MakeSignalDecoderPacket(packet, i, static_cast<SignalAspects>(aspect % 32));

		uint16_t address;
		SignalAspects packetAspect;

		std::tie(address, packetAspect) = ExtractSignalDataFromPacket(packet);

		ASSERT_EQ(address, i);
		ASSERT_EQ(packet[2], aspect % 32);
		ASSERT_EQ(packetAspect, static_cast<SignalAspects>(packet[2]));

	}
}