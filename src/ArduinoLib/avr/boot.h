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

inline uint8_t boot_signature_byte_get(uint16_t address)
{
	//just random numbers...
	uint8_t buffer[16] = {	0xFC, 0xAA, 0xBB, 0xFF, 0xB0, 0xB1, 0xB2, 0xB3,
							0xA4, 0xB5, 0xC6, 0xD7, 0xE8, 0xF9, 0xAA, 0xBC };

	return buffer[address - 0x0E];
}
