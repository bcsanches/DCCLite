// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Serial.h"

#include <stdio.h>

SerialImpl Serial;

void SerialImpl::begin(int frequency)
{
	//nothing todo...
}

void SerialImpl::print(const char *str)
{
	printf("%s", str);
}

void SerialImpl::print(int value, int base)
{
	printf(base == 10 ? "%d" : "%X", value);
}

void SerialImpl::print(unsigned int value, int base)
{
	printf(base == 10 ? "%u" : "%X", value);
}

void SerialImpl::print(char value)
{
	printf("%c", value);
}

void SerialImpl::println(const char *str)
{
	printf("%s\n", str);
}

int SerialImpl::available()
{
	return m_strData.length() - m_uPos;
}

int SerialImpl::read()
{		
	return m_uPos >= m_strData.length() ? -1 : m_strData[m_uPos++];
}

void SerialImpl::flush()
{
	//empty
}
