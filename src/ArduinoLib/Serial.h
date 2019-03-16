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

#include <string>

#include "ArduinoLibDefs.h"

struct ARDUINO_API SerialImpl
{
	void begin(int frequency);

	void print(const char *str);
	void print(int value, int base = 10);
	void print(unsigned int value, int base = 10);
	void print(char value);
	void println(const char *str);

	int available();
	int read();

	void flush();

	inline void internalSetData(const char *data)
	{
		m_uPos = 0;
		m_strData.assign(data);
	}

	private:
#pragma warning(disable:4251)
		std::string m_strData;
#pragma warning(default:4251)

		size_t		m_uPos = 0;


};

ARDUINO_API extern SerialImpl Serial;


