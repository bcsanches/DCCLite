// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NmraUtil.h"

#include "fmt/format.h"
#include "magic_enum.hpp"

/*
	Straight from JMRI source code (https://www.jmri.org/), only used for testing purposes

*/

static void accSignalDecoderPktCommon(uint8_t dest[4], const int lowAddr, const int boardAddr, const dcclite::SignalAspects aspect)
{

	int midAddr = boardAddr & 0x7F;
	int highAddr = ((~boardAddr) >> 6) & 0x07;
	
	dest[0] = (uint8_t)(0x80 | midAddr);
	dest[1] = (uint8_t)(0x01 | (highAddr << 4) | (lowAddr << 1));
	dest[2] = (uint8_t)(0x1F & static_cast<uint8_t>(aspect));
	dest[3] = (uint8_t)(dest[0] ^ dest[1] ^ dest[2]);
}

void dcclite::MakeSignalDecoderPacket(uint8_t dest[4], uint16_t outputAddr, const dcclite::SignalAspects aspect)
{
	outputAddr -= 1; // Make the address 0 based
	int lowAddr = (outputAddr & 0x03);  // Output Pair Address
	int boardAddr = (outputAddr >> 2); // Board Address

	accSignalDecoderPktCommon(dest, lowAddr, boardAddr, aspect);
}

std::tuple<uint16_t, dcclite::SignalAspects> dcclite::ExtractSignalDataFromPacket(const uint8_t packet[3])
{
	int midAddr = packet[0] & 0x3f;
	int lowAddr = (packet[1] & 0x0E) >> 1;
	int hiAddr = ((~packet[1]) & 0x70) >> 4;

	int boardAddr = (hiAddr << 6 | midAddr);

	return std::make_tuple(((boardAddr << 2) | lowAddr) + 1, static_cast<dcclite::SignalAspects>(packet[2]));
}

dcclite::SignalAspects dcclite::ConvertNameToAspect(const char *name)
{
	auto v = magic_enum::enum_cast<SignalAspects>(name);

	if (!v.has_value())
	{
		throw std::invalid_argument(fmt::format("[dcclite::ConvertNameToAspect] Name {} is not a valid aspect", name).c_str());
	}

	return v.value();
}


std::tuple<int16_t, uint16_t> dcclite::ConvertAddressToNMRA(uint16_t address)
{
	return std::make_tuple(address >> 2, address & 3);
}

