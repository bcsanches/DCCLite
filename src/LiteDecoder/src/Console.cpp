#include "Console.h"

#include <stdarg.h>
#include <stdio.h>

#include <Arduino.h>

#include "Parser.h"

#include "LiteDecoder.h"
#include "NetUdp.h"
#include "Session.h"
#include "Storage.h"

//hack for emulator for avoiding polutating namespace with stupid names
#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

void Console::Init() 
{
    Serial.begin(9600);
    Serial.flush();

    Serial.print("LiteDecoder ");
    Serial.print(ARDUINO_TYPE);
    Serial.print(" / ");    
    Serial.print(VERSION);
    Serial.print(" / ");
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
	Console::SendLogEx("[CONSOLE]", "in:", ' ', command);

	if(strncmp(command, "cfg", 3) == 0)
	{
		//format: cfg <nodeName> <mac> <port> <srvipv4>	<srvport>	

		dcclite::Parser parser(command+3);

		char nodeName[17];
		if(parser.GetToken(nodeName, sizeof(nodeName)) != dcclite::TOKEN_ID)
		{
			Console::SendLogEx("[CONSOLE]", "NOK", ' ', "node", ' ', "name");

			return;
		}

		//Console::SendLog("[CONSOLE]", "name: %s", nodeName);
		
		uint8_t mac[6];
		for(int i = 0;i < 6; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLogEx("[CONSOLE]", "NOK", ' ', "mac");

				return;
			}

			//Console::SendLog("[CONSOLE]", "mac: %d", number);

			mac[i] = number;

			if(i == 5)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{				
				Console::SendLogEx("[CONSOLE]", "NOK", ' ', "mac", ' ', "sep");

				return;
			}
		}

		int port;
		if(parser.GetNumber(port) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLogEx("[CONSOLE]", "NOK", ' ', "port");

			return;
		}

		//Console::SendLog("[CONSOLE]", "port: %d", port);

		uint8_t ip[4];
		for(int i = 0;i < 4; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLogEx("[CONSOLE]", "NOK", ' ', "ip");

				return;
			}
			ip[i] = number; 		

			//Console::SendLog("[CONSOLE]", "ip: %d", number);

			if(i == 3)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{
				Console::SendLogEx("[CONSOLE]", "NOK", ' ', "ip", ' ', "sep");

				return;
			}
		}

		int srvport;
		if (parser.GetNumber(srvport) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLogEx("[CONSOLE]", "NOK", ' ', "srvport");

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
	else
	{
		Console::SendLogEx("[CONSOLE]", "NOK", ' ', command);
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