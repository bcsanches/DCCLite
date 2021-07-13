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

#include "LocalDecoderManager.h"
#include "SensorDecoder.h"
#include "ServoTurnoutDecoder.h"

#include "Console.h"
#include "Storage.h"

constexpr auto MAX_ROUTES = 15;

static unsigned long g_uLastFrameTime = 0;

bool Console::Custom_ParseCommand(const char *command)
{
	return false;
}

bool Storage::Custom_LoadModules(const Storage::Lump &lump, EpromStream &stream)
{
	return false;
}

void Storage::Custom_SaveModules(EpromStream &stream)
{

}

void setup()
{		
	Console::Init();

	{
		auto turnout = LocalDecoderManager::CreateServoTurnout(
			dcclite::SRVT_INVERTED_OPERATION | dcclite::SRVT_INVERTED_POWER,	//flags
			{ 10 },																//pin
			25,																	//range
			20,																	//ticks
			{7},																//powerPin
			dcclite::NullPin													//frogPin
		);

		auto sensor1 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags
			{ 9 },											//pin
			0,												//activate Delay (msec)
			0												//deactivate Delay (msec)
		);

		LocalDecoderManager::CreateButton(*sensor1, *turnout, LocalDecoderManager::kTOGGLE);

		auto sensor2 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags
			{ 11 },											//pin
			0,												//activate Delay (msec)
			0												//deactivate Delay (msec)
		);

		LocalDecoderManager::CreateButton(*sensor2, *turnout, LocalDecoderManager::kTHROW);

		auto sensor3 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags
			{ 12 },											//pin
			0,												//activate Delay (msec)
			0												//deactivate Delay (msec)
		);

		LocalDecoderManager::CreateButton(*sensor3, *turnout, LocalDecoderManager::kCLOSE);
	}

	g_uLastFrameTime = millis();
}

void loop() 
{		
	auto currentTime = millis();	

	auto ticks = currentTime - g_uLastFrameTime;
	if (ticks == 0)
		return;	

	LocalDecoderManager::Update(ticks);	
}
