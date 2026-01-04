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

#include <dcclite/Log.h>

SerialImpl Serial;

static char		g_szBuffer[8192 * 4];
static unsigned	g_uPosition = 0;
static unsigned g_uDataPosition = 0;
static unsigned g_uLineBreakPosition = 0;

static void FlushBufferLines(std::optional<unsigned> hint = std::nullopt)
{	
	g_uLineBreakPosition = (hint.has_value() && hint.value() < g_uPosition) ? hint.value() : g_uLineBreakPosition;

	//search for a line break...
	for(; g_uLineBreakPosition < g_uPosition; ++g_uLineBreakPosition)
	{
		if (g_szBuffer[g_uLineBreakPosition] != '\n')
			continue;		
		
		//found a line break, flush until here
		std::string_view outputData(g_szBuffer + g_uDataPosition, g_uLineBreakPosition - g_uDataPosition);
		dcclite::Log::Info("[ArduinoSerial] {}", outputData);

		//advance data so we dont flush it again...
		g_uDataPosition = g_uLineBreakPosition + 1;
	}
}

static void FlushBufferRemaining()
{
	//flush all lines
	FlushBufferLines();

	//any data without line break?
	if (g_uDataPosition < g_uPosition)
	{
		//flush remaining data
		std::string_view outputData(g_szBuffer + g_uDataPosition, g_uPosition);
		dcclite::Log::Info("[ArduinoSerial] {} [UNTERMINATED]", outputData);
	}

	g_uDataPosition = g_uPosition = g_uLineBreakPosition = 0;
}

void SerialImpl::internalFlushBufferRemaining()
{
	FlushBufferRemaining();
}

void SerialImpl::begin(int frequency)
{
	//nothing todo...
}

void SerialImpl::print(const char *str)
{
	g_uPosition += sprintf(g_szBuffer + g_uPosition, "%s", str);

	//we may have a \n embedded on str, so try to flush lines
	FlushBufferLines();
}

void SerialImpl::print(int value, int base)
{
	g_uPosition += sprintf(g_szBuffer + g_uPosition, base == 10 ? "%d" : "%X", value);
}

void SerialImpl::print(unsigned int value, int base)
{
	g_uPosition += sprintf(g_szBuffer + g_uPosition, base == 10 ? "%u" : "%X", value);
}

void SerialImpl::print(unsigned long value, int base)
{
	g_uPosition += sprintf(g_szBuffer + g_uPosition, base == 10 ? "%u" : "%X", value);
}

void SerialImpl::print(char value)
{
	g_szBuffer[g_uPosition++] = value;

	if(value == '\n')
		FlushBufferLines(g_uPosition - 1);
}

void SerialImpl::println(const char *str)
{
	g_uPosition += sprintf(g_szBuffer + g_uPosition, "%s\n", str);

	//we dont hint here, str may contain \n embedded...
	FlushBufferLines();
}

void SerialImpl::println()
{
	g_szBuffer[g_uPosition++] = '\n';

	//this should be the only \n so far...
	FlushBufferLines(g_uPosition - 1);
}

int SerialImpl::available()
{
	return static_cast<int>(m_strData.length() - m_uPos);
}

int SerialImpl::read()
{		
	return m_uPos >= m_strData.length() ? -1 : m_strData[m_uPos++];
}

void SerialImpl::write(char value)
{
	putc(value, stdout);	
}

void SerialImpl::flush()
{
	//empty
}
