// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Console.h"

#include <stdarg.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <Arduino.h>

#include "Parser.h"

#include "ArduinoTypes.h"
#include "SharedLibDefs.h"
#include "Strings.h"

//hack for emulator for avoiding polutating namespace with stupid names
#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

const char ConsoleModuleName[] PROGMEM = {"CONSOLE"} ;
#define MODULE_NAME Console::FlashStr(ConsoleModuleName)

const char CmdMemName[] PROGMEM = {"mem"} ;

extern int __heap_start, *__brkval;

void Console::Init() 
{
    Serial.begin(9600);
    Serial.flush();

    Serial.print(F("LiteDecoder "));
    Serial.print(ARDUINO_TYPE);
    Serial.print(F(" / "));    
    Serial.print(F(DCCLITE_VERSION));
    Serial.print(F(" / "));
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);    
}


void Console::Send(const char *str)
{
    Serial.print(str);
}

void Console:: SendLn(const char *str)
{
    Serial.println(str);
}

void Console::Send(int value, Format format)
{
    Serial.print(value, format == Format::DECIMAL ? DEC : HEX);
}

void Console::Send(unsigned int value, Format format)
{
	Serial.print(value, format == Format::DECIMAL ? DEC : HEX);
}

void Console::Send(char value)
{
	Serial.print(value);
}

int Console::Available()
{
    return Serial.available();
}

int Console::ReadChar()
{
    return Serial.read();
}

static void Parse(const char *command)
{
	Console::SendLogEx(MODULE_NAME, "in:", " ", command);

	if(strncmp_P(command, CmdMemName, 3) == 0)
	{
		//based on https://github.com/DccPlusPlus/BaseStation/blob/master/DCCpp_Uno/SerialCommand.cpp

#ifndef WIN32
		int v; 
		Console::SendLogEx(MODULE_NAME, (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));		
#else
		Console::SendLogEx(MODULE_NAME, "LOTS LOTS LOTS");
#endif

	}
    else if(!Console::Custom_ParseCommand(command))
	{
		Console::SendLogEx(MODULE_NAME, FSTR_NOK, ' ', command);
	}
}


constexpr auto MAX_COMMAND_LENGTH = 65;

void Console::Update()
{
	static char command[MAX_COMMAND_LENGTH + 1];
    static int pos = 0;	

    while(Console::Available() > 0)
    {
        char c = Console::ReadChar();		

        if(c == '/')
        {
          pos = 0;
          command[pos] = 0;
        }
        else if(c == ';')
        {
            command[pos] = 0;
            Parse(command);

			pos = 0;
        }
        //if we are overflowing, we simple skip characters
        else if(pos < MAX_COMMAND_LENGTH)
        {
            command[pos] = c;
            ++pos;
        }
    }
}