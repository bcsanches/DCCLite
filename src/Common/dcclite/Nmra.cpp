// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Nmra.h"

#include "fmt/format.h"
#include "magic_enum/magic_enum.hpp"

/*
	Straight from JMRI source code (https://www.jmri.org/), only used for testing purposes

*/

/*
* 
* NMRA S-9.2.1
* 
*		Byte 1		Byte 2		Byte 3		Byte 4
		10AAAAAA	0AAA0AA1	XXXXXXXX	EEEEEEEE
		Address		Address		Aspect		Checksum

		0x3F -> 00111111, so extract the 6 bits of the address from the first byte
		0x0E -> 00001110, so extract the 3 bits of the address from the second byt, shift right 1 to get the 3 bits of the address
		0x70 -> 01110000, so extract the 3 bits of the address from the second byte, shift right 4 to get the 3 bits of the address
			invert it as they are 2 complemented, so we can get the 3 bits of the address from the second byte

*/

static void accSignalDecoderPktCommon(uint8_t dest[4], const int lowAddr, const int boardAddr, const dcclite::SignalAspects aspect)
{
	int midAddr = boardAddr & 0x3F;
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

	//OutputAddress = (BoardAddress * 4) + LowAddress + 1		
	return std::make_tuple(((boardAddr << 2) | lowAddr) + 1, static_cast<dcclite::SignalAspects>(packet[2]));
}

std::optional<dcclite::SignalAspects> dcclite::TryConvertNameToAspect(const char *name)
{
	return magic_enum::enum_cast<SignalAspects>(name);
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

std::string_view dcclite::ConvertAspectToName(const SignalAspects aspect)
{
	return magic_enum::enum_name(aspect);
}


std::tuple<int16_t, uint16_t> dcclite::ConvertAddressToNMRA(uint16_t address)
{
	return std::make_tuple(address >> 2, address & 3);
}

