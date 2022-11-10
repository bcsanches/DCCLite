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
#include "Printf.h"
#include "SharedLibDefs.h"
#include "Strings.h"

//hack for emulator for avoiding polutating namespace with stupid names
#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

#define MODULE_NAME F("CONSOLE")

extern int __heap_start;
extern char *__brkval;

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

static void Parse(const char *command)
{
	//Console::SendLogEx(MODULE_NAME, "in:", " ", command);
    //DCCLITE_LOG << MODULE_NAME << "in: " << command << DCCLITE_ENDL;        
    //Console::Printf(F("%z: in %s\n"), MODULE_NAME, command);

	if(FStrNCmp(command, F("mem"), 3) == 0)
	{
		//based on https://github.com/DccPlusPlus/BaseStation/blob/master/DCCpp_Uno/SerialCommand.cpp

#ifndef WIN32
		int v; 
		//Console::SendLogEx(MODULE_NAME, (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));		
        DCCLITE_LOG_MODULE_LN((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
        //Console::Printf(F("%z: %d\n"), MODULE_NAME, (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
        
#else
		//Console::SendLogEx(MODULE_NAME, "LOTS LOTS LOTS");
        DCCLITE_LOG_MODULE_LN("LOTS LOTS LOTS");
#endif

	}
    else if(!Console::Custom_ParseCommand(command))
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_NOK, ' ', command);
        DCCLITE_LOG_MODULE_LN(FSTR_NOK << ' ' << command);
        //Console::Printf(F("%z: %z %s"), FSTR_NOK, command);
	}
}

class FlashStringWrapper
{
    public:
        inline FlashStringWrapper(const Console::ConsoleFlashStringHelper_t *str) noexcept :
            m_fszStr{ str }
        {
            //empty
        }

        inline const char ReadChar(unsigned pos) noexcept
        {
            return FStrReadChar(m_fszStr, pos);
        }

    private:
        const Console::ConsoleFlashStringHelper_t *m_fszStr;
};

class SerialWrapper
{
    public:

        void Println()
        {
            Serial.println();
        }
        
        void Print(const char *str)
        {
            Serial.print(str);
        }

        void PrintFlash(const char *str)
        {
            auto *str2 = (Console::ConsoleFlashStringHelper_t *) str;

            Serial.print(str2);
        }

        void Print(int n)
        {
            Serial.print(n);
        }

        void Print(unsigned n)
        {
            Serial.print(n);
        }

        void Print(char ch)
        {
            Serial.print(ch);
        }
};

void Console::Printf(const ConsoleFlashStringHelper_t *format, ...)
{
    va_list args;
    va_start(args, format);        

     dcclite::PrintfImpl(SerialWrapper{}, FlashStringWrapper{ format }, args);

    va_end(args);
}

constexpr auto MAX_COMMAND_LENGTH = 65;

void Console::Update()
{
	static char command[MAX_COMMAND_LENGTH + 1];
    static int pos = 0;	

    while(Serial.available() > 0)
    {
        char c = Serial.read();		

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