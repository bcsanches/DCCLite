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
#include "NetUdp.h"
#include "Session.h"
#include "Storage.h"

static unsigned long g_uStartTime = 0;
static unsigned long g_uFrameCount = 0;
static float g_uFps = 0;

const int onboardLedPin = 13;

bool g_fNetReady = false;

void setup()
{
	Console::Init();
	Blinker::Init();

	Storage::LoadConfig();

	g_fNetReady = NetUdp::Init(Session::GetReceiverCallback());

	g_uStartTime = millis();

	Blinker::Play(Blinker::Animations::OK);	

	Session::Init();

	Console::SendLogEx("setup", "done");
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

	if (g_fNetReady)
	{
		NetUdp::Update();
		Session::Update();
	}
}
