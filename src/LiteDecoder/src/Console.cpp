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

#include "LiteDecoder.h"
#include "NetUdp.h"
#include "Session.h"
#include "Storage.h"
#include "Strings.h"

//hack for emulator for avoiding polutating namespace with stupid names
#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

const char ConsoleModuleName[] PROGMEM = {"[CONSOLE]"} ;
#define MODULE_NAME Console::FlashStr(ConsoleModuleName)

const char CmdDumpName[] PROGMEM = {"dump"} ;
const char CmdHDumpName[] PROGMEM = {"hdump"} ;

void Console::Init() 
{
    Serial.begin(9600);
    Serial.flush();

    Serial.print(F("LiteDecoder "));
    Serial.print(ARDUINO_TYPE);
    Serial.print(F(" / "));    
    Serial.print(VERSION);
    Serial.print(F(" / "));
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);    
}


void Console::Send(const char *str)
{
    Serial.print(str);
}

void Console::SendLn(const char *str)
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

	if(strncmp(command, "cfg", 3) == 0)
	{
		//format: cfg <nodeName> <mac> <port> <srvipv4>	<srvport>	

		dcclite::Parser parser(command+3);

		char nodeName[17];
		if(parser.GetToken(nodeName, sizeof(nodeName)) != dcclite::TOKEN_ID)
		{
			Console::SendLogEx(MODULE_NAME, "NOK", " ", FSTR_NODE, " ", FSTR_NAME);

			return;
		}

		//Console::SendLog("[CONSOLE]", "name: %s", nodeName);
		
		uint8_t mac[6];
		for(int i = 0;i < 6; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLogEx(MODULE_NAME, "NOK", " ", "mac");

				return;
			}

			//Console::SendLog("[CONSOLE]", "mac: %d", number);

			mac[i] = number;

			if(i == 5)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{				
				Console::SendLogEx(MODULE_NAME, "NOK", " ", "mac", " ", "sep");

				return;
			}
		}

		int port;
		if(parser.GetNumber(port) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLogEx(MODULE_NAME, "NOK", " ", FSTR_PORT);

			return;
		}

		//Console::SendLog("[CONSOLE]", "port: %d", port);

		uint8_t ip[4];
		for(int i = 0;i < 4; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLogEx(MODULE_NAME, "NOK", " ", "ip");

				return;
			}
			ip[i] = number; 		

			//Console::SendLog("[CONSOLE]", "ip: %d", number);

			if(i == 3)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{
				Console::SendLogEx(MODULE_NAME, "NOK", " ", "ip", " ", "sep");

				return;
			}
		}

		int srvport;
		if (parser.GetNumber(srvport) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLogEx(MODULE_NAME, "NOK", " ", FSTR_SRVPORT);

			return;
		}

		NetUdp::Configure(nodeName, port, mac);
		Session::Configure(ip, srvport);

		Console::SendLn("ok");
	}
	else if(strncmp(command, "sv", 2) == 0)
	{
		Storage::SaveConfig();
		Console::SendLn("ok");
	}
	else if(strncmp_P(command, CmdDumpName, 4) == 0)
	{
		Storage::Dump();
		Console::SendLn("ok");
	}
	else if(strncmp_P(command, CmdHDumpName, 5) == 0)
	{
		Storage::DumpHex();
		Console::SendLn("ok");
	}
	else
	{
		Console::SendLogEx(MODULE_NAME, "NOK", ' ', command);
	}
}


#define MAX_COMMAND_LENGTH 65

void Console::Update()
{
	static char command[MAX_COMMAND_LENGTH + 1];
    static int pos = 0;

	char c;

    while(Console::Available() > 0)
    {
        c = Console::ReadChar();		

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