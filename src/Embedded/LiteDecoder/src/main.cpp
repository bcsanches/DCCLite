// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <Arduino.h>

#include "main.h"
#include "Blinker.h"
#include "Console.h"
#include "DecoderManager.h"
#include "NetUdp.h"
#include "Parser.h"
#include "Session.h"
#include "Storage.h"
#include "Strings.h"

static unsigned long g_uStartTime = 0;
static unsigned long g_uFrameCount = 0;
static float g_uFps = 0;

bool g_fNetReady = false;

const char CmdCfgName[] PROGMEM = { "cfg" };
const char CmdDumpName[] PROGMEM = { "dump" };
const char CmdHDumpName[] PROGMEM = { "hdump" };
const char MainModuleName[] PROGMEM = { "LITE_DECODER" };
#define MODULE_NAME FlashStr(MainModuleName)


bool Console::ParseCustomCommand(const char *command)
{
	if (strncmp_P(command, CmdCfgName, 3) == 0)
	{
		//format: cfg <nodeName> <mac> <port> <srvport>	

		dcclite::Parser parser(command + 3);

		char nodeName[17];
		if (parser.GetToken(nodeName, sizeof(nodeName)) != dcclite::Tokens::ID)
		{
			Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_NODE, " ", FSTR_NAME);

			return true;
		}

		//Console::SendLog("[CONSOLE]", "name: %s", nodeName);

		uint8_t mac[6];
		for (int i = 0; i < 6; ++i)
		{
			int number;
			if (parser.GetNumber(number) != dcclite::Tokens::NUMBER)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", "mac");

				return true;
			}

			//Console::SendLog("[CONSOLE]", "mac: %d", number);

			mac[i] = number;

			if (i == 5)
				break;

			if (parser.GetToken(nullptr, 0) != dcclite::Tokens::DOT)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", "mac", " ", "sep");

				return true;
			}
		}

		int port;
		if (parser.GetNumber(port) != dcclite::Tokens::NUMBER)
		{
			Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_PORT);

			return true;
		}

		uint8_t ip[4] = { 0 };
		int srvport;
		if (parser.GetNumber(srvport) != dcclite::Tokens::NUMBER)
		{
			Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_SRVPORT);

			return true;
		}

		NetUdp::Configure(nodeName, port, mac);
		Session::Configure(ip, srvport);

		Console::SendLogEx(MODULE_NAME, FSTR_OK);

		Storage::SaveConfig();
		Console::SendLogEx(MODULE_NAME, FSTR_OK);
	}
	else if (strncmp_P(command, CmdDumpName, 4) == 0)
	{
		Storage::Dump();
		Console::SendLogEx(MODULE_NAME, FSTR_OK);

		return true;
	}
	else if (strncmp_P(command, CmdHDumpName, 5) == 0)
	{
		Storage::DumpHex();
		Console::SendLogEx(MODULE_NAME, FSTR_OK);

		return true;
	}

	//unkwnon cmd
	return false;
}

void setup()
{
	Console::Init();
	Blinker::Init();

	const int decodersStoragePos = Storage::LoadConfig();

	g_fNetReady = NetUdp::Init(Session::GetReceiverCallback());

	g_uStartTime = millis();

	Blinker::Play(Blinker::Animations::OK);		

	if(decodersStoragePos > 0)
	{
		Storage::LoadDecoders(decodersStoragePos);
	}

	Console::SendLogEx(FSTR_SETUP, " ", FSTR_OK);
}

void loop() 
{	
	++g_uFrameCount;

	auto currentTime = millis();
	int seconds = 0;

	while((currentTime - g_uStartTime) >= 1000)
	{
		++seconds;
		g_uStartTime += 1000;
	}

	if(seconds > 0)
	{
		g_uFps = g_uFrameCount / static_cast<float>(seconds);
		g_uFrameCount = 0;

		//Console::sendLog("main", "fps %d", (int)g_fps);
	}

	Console::Update();
	Blinker::Update();
	const bool stateChangeDetected = DecoderManager::Update(currentTime);

	if (g_fNetReady)
	{
		NetUdp::Update();
		Session::Update(currentTime, stateChangeDetected);
	}
}