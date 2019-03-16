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

#include <array>

#include "ArduinoLibDefs.h"

struct ARDUINO_API EEPROMImpl
{
	void put(size_t pos, const void *ptr, size_t len);
	void get(size_t pos, void *ptr, size_t len);

	template<typename T>
	inline void put(size_t pos, T &data)
	{
		put(pos, &data, sizeof(T));
	}

	template <typename T>
	inline void get(size_t pos, T &data)
	{
		get(pos, &data, sizeof(T));
	}

	size_t length();

	unsigned char read(size_t pos);	
};

ARDUINO_API extern EEPROMImpl EEPROM;




