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

#include <limits.h>

#include "main.h"
#include "Blinker.h"
#include "Console.h"
#include "DecoderManager.h"
#include "Misc.h"
#include "NetUdp.h"
#include "Parser.h"
#include "Session.h"
#include "Storage.h"
#include "Strings.h"

static unsigned long g_uStartTime = 0;
static unsigned long g_uFrameCount = 0;
static float g_uFps = 0;

static unsigned short g_uDecodersPosition = 0;

static_assert(sizeof(g_uDecodersPosition) == 2);

bool g_fNetReady = false;

#define CMD_CFG_NAME F("cfg")
#define CMD_DUMP_NAME F("dump")
#define CMD_HDUMP_NAME F("hdump")

#define MODULE_NAME F("MAIN")

bool Console::Custom_ParseCommand(const char *command)
{
	if (FStrNCmp(command, CMD_CFG_NAME, 3) == 0)
	{
		//format: cfg <nodeName> <mac> <port> <srvport>	

		dcclite::Parser parser(command + 3);

		char nodeName[17];
		if (parser.GetToken(nodeName, sizeof(nodeName)) != dcclite::Tokens::ID)
		{
			//Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_NODE, " ", FSTR_NAME);
			DCCLITE_LOG_MODULE_LN(FSTR_NOK << ' ' << FSTR_NODE << ' ' << FSTR_NAME);

			return true;
		}

		//Serial.println(F("etst"));

		//Console::SendLog("[CONSOLE]", "name: %s", nodeName);

		uint8_t mac[6];
		for (int i = 0; i < 6; ++i)
		{
			int number;
			if (parser.GetNumber(number) != dcclite::Tokens::NUMBER)
			{
				//Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", "mac");
				DCCLITE_LOG << MODULE_NAME << FSTR_NOK << F(" mac") << DCCLITE_ENDL;

				return true;
			}

			//Console::SendLog("[CONSOLE]", "mac: %d", number);

			mac[i] = number;

			if (i == 5)
				break;

			if (parser.GetToken(nullptr, 0) != dcclite::Tokens::DOT)
			{
				//Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", "mac", " ", "sep");
				DCCLITE_LOG << MODULE_NAME << FSTR_NOK << F(" mac sep") << DCCLITE_ENDL;

				return true;
			}
		}

		int port;
		if (parser.GetNumber(port) != dcclite::Tokens::NUMBER)
		{
			//Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_PORT);
			DCCLITE_LOG << MODULE_NAME << FSTR_NOK << ' ' << FSTR_PORT << DCCLITE_ENDL;

			return true;
		}

		uint8_t ip[4] = { 0 };
		int srvport;
		if (parser.GetNumber(srvport) != dcclite::Tokens::NUMBER)
		{
			//Console::SendLogEx(MODULE_NAME, FSTR_NOK, " ", FSTR_SRVPORT);
			DCCLITE_LOG << MODULE_NAME << FSTR_NOK << ' ' << FSTR_SRVPORT << DCCLITE_ENDL;

			return true;
		}

		NetUdp::Configure(nodeName, port, mac);
		Session::Configure(ip, srvport);

		//Console::SendLogEx(MODULE_NAME, FSTR_OK);
		DCCLITE_LOG_MODULE_LN(FSTR_OK);

		Storage::SaveConfig();

		//Console::SendLogEx(MODULE_NAME, FSTR_OK);
		DCCLITE_LOG_MODULE_LN(FSTR_OK);

		return true;
	}
	else if (FStrNCmp(command, CMD_DUMP_NAME, 4) == 0)
	{
		Storage::Dump();

		//Console::SendLogEx(MODULE_NAME, FSTR_OK);
		DCCLITE_LOG_MODULE_LN(FSTR_OK);

		return true;
	}
	else if (FStrNCmp(command, CMD_HDUMP_NAME, 5) == 0)
	{
		Storage::DumpHex();

		//Console::SendLogEx(MODULE_NAME, FSTR_OK);
		DCCLITE_LOG_MODULE_LN(FSTR_OK);

		return true;
	}

	//unkwnon cmd
	return false;
}

#define DECODERS_STORAGE_ID F("DECS016")
#define NET_UDP_STORAGE_ID	F("NetU002")
#define SESSION_STORAGE_ID  F("Sson001")

bool Storage::Custom_LoadModules(const Storage::Lump &lump, Storage::EpromStream &stream)
{
	if (FStrNCmp(lump.m_archName, DECODERS_STORAGE_ID, FStrLen(DECODERS_STORAGE_ID)) == 0)
	{
		//make sure below cast will not fail...
		static_assert(sizeof(Storage::Lump) < 255);

		g_uDecodersPosition = stream.GetIndex() - static_cast<unsigned short>(sizeof(Storage::Lump));
		stream.Skip(lump.m_uLength);

		//Console::SendLogEx(MODULE_NAME, FSTR_DECODERS, ' ', "cfg", ' ', g_uDecodersPosition);
		DCCLITE_LOG_MODULE_LN(FSTR_DECODERS << F(" cfg ") << g_uDecodersPosition);

		return true;
	}

	if (FStrNCmp(lump.m_archName, NET_UDP_STORAGE_ID, FStrLen(NET_UDP_STORAGE_ID)) == 0)
	{
		//Console::SendLogEx(MODULE_NAME, "net", "udp", ' ', "cfg");
		DCCLITE_LOG_MODULE_LN(MODULE_NAME << F(" net udp cfg"));

		NetUdp::LoadConfig(stream);

		return true;
	}

	if (FStrNCmp(lump.m_archName, SESSION_STORAGE_ID, FStrLen(SESSION_STORAGE_ID)) == 0)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_SESSION, ' ', "cfg");
		DCCLITE_LOG_MODULE_LN(FSTR_SESSION << F(" cfg"));

		Session::LoadConfig(stream);

		return true;
	}

	return false;
}

void Storage::Custom_SaveModules(Storage::EpromStream &stream)
{
	{
		LumpWriter netLump(stream, NET_UDP_STORAGE_ID);

		NetUdp::SaveConfig(stream);
	}

	{
		LumpWriter sessionLump(stream, SESSION_STORAGE_ID);

		Session::SaveConfig(stream);
	}

	{
		LumpWriter decodersLump(stream, DECODERS_STORAGE_ID);

		DecoderManager::SaveConfig(stream);
	}
}

void Storage_LoadDecoders(uint32_t position)
{
	Storage::EpromStream stream(position);

	Storage::Lump lump;

	stream.GetString(lump.m_archName, sizeof(lump.m_archName));
	stream.Get(lump.m_uLength);

	if (FStrNCmp(lump.m_archName, DECODERS_STORAGE_ID, FStrLen(DECODERS_STORAGE_ID)) != 0)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_UNKNOWN, ' ', FSTR_LUMP, ' ', lump.m_archName);
		DCCLITE_LOG << MODULE_NAME << FSTR_UNKNOWN << ' ' << FSTR_LUMP << ' ' << lump.m_archName << DCCLITE_ENDL;

		return;
	}

	//Console::SendLogEx(MODULE_NAME, F("Loading config"));
	DCCLITE_LOG_MODULE_LN(F("Loading config"));	

	DecoderManager::LoadConfig(stream);
}

void setup()
{
	Console::Init();
	Blinker::Init();

	Storage::LoadConfig();

	g_fNetReady = NetUdp::Init(Session::GetReceiverCallback());

	g_uStartTime = millis();

	Blinker::Play(Blinker::Animations::OK);		

	if(g_uDecodersPosition > 0)
	{
		Storage_LoadDecoders(g_uDecodersPosition);
	}

	//Console::SendLogEx(FSTR_SETUP, " ", FSTR_OK);
	DCCLITE_LOG_MODULE_LN(FSTR_SETUP << ' ' << FSTR_OK);
}

static int g_iMinHeapSpace = INT_MAX;

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

		//Console::SendLogEx("main", "fps ", (int)g_uFps);

		auto freeSpace = dcclite::GetHeapFreeSpace();
		if (freeSpace < g_iMinHeapSpace)
		{
			g_iMinHeapSpace = freeSpace;
			Session::UpdateFreeRam(g_iMinHeapSpace);

			//Console::SendLogEx(MODULE_NAME, F("ram "), g_iMinHeapSpace, F(" | fps "), (int)g_uFps);			
			DCCLITE_LOG << MODULE_NAME << F("ram ") << g_iMinHeapSpace << F(" | fps ") << (int)g_uFps << DCCLITE_ENDL;
		}
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
