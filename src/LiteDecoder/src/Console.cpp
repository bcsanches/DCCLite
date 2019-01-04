#include "Console.h"

#include <stdarg.h>
#include <stdio.h>

#include <Arduino.h>

#include "Parser.h"

#include "LiteDecoder.h"
#include "NetUdp.h"
#include "Session.h"
#include "Storage.h"

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

void Console::SendLog(const char *module, const char *format, ...)
{
    char buffer[128];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, 128, format, args);

    Serial.println("");
    Serial.print("<LOG ");
    Serial.print(module);
    Serial.print(" ");

    Serial.print(buffer);

    Serial.println(">");

    va_end(args);
}

void Console::Send(const char *str)
{
    Serial.print(str);
}

void Console::SendLn(const char *str)
{
    Serial.println(str);
}

void Console::Send(int value)
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
	Console::SendLog("[CONSOLE]", "in: %s", command);

	if(strncmp(command, "cfg", 3) == 0)
	{
		//format: cfg <nodeName> <mac> <port> <srvipv4>	<srvport>	

		dcclite::Parser parser(command+3);

		char nodeName[17];
		if(parser.GetToken(nodeName, sizeof(nodeName)) != dcclite::TOKEN_ID)
		{
			Console::SendLog("[CONSOLE]", "NOK node name");

			return;
		}

		//Console::SendLog("[CONSOLE]", "name: %s", nodeName);
		
		uint8_t mac[6];
		for(int i = 0;i < 6; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLog("[CONSOLE]", "NOK mac");

				return;
			}

			//Console::SendLog("[CONSOLE]", "mac: %d", number);

			mac[i] = number;

			if(i == 5)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{				
				Console::SendLog("[CONSOLE]", "NOK mac sep");

				return;
			}
		}

		int port;
		if(parser.GetNumber(port) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLog("[CONSOLE]", "NOK port");

			return;
		}

		//Console::SendLog("[CONSOLE]", "port: %d", port);

		uint8_t ip[4];
		for(int i = 0;i < 4; ++i)
		{
			int number;
			if(parser.GetNumber(number) != dcclite::TOKEN_NUMBER)
			{
				Console::SendLog("[CONSOLE]", "NOK ip");

				return;
			}
			ip[i] = number; 		

			//Console::SendLog("[CONSOLE]", "ip: %d", number);

			if(i == 3)
				break;

			if(parser.GetToken(nullptr, 0) != dcclite::TOKEN_DOT)
			{
				Console::SendLog("[CONSOLE]", "NOK ip sep");

				return;
			}
		}

		int srvport;
		if (parser.GetNumber(srvport) != dcclite::TOKEN_NUMBER)
		{
			Console::SendLog("[CONSOLE]", "NOK srvport");

			return;
		}

		NetUdp::Configure(nodeName, port, mac);
		Session::Configure(ip, srvport);

		Console::SendLn("OK");
	}
	else if(strncmp(command, "sv", 2) == 0)
	{
		Storage::SaveConfig();
		Console::SendLn("OK");
	}
	else
	{
		Console::SendLog("[CONSOLE]", "NOK cmd");
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