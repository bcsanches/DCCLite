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

#include "ServoTurnoutDecoder.h"

constexpr auto MAX_DECODERS = 14;

static unsigned long g_uLastFrameTime = 0;
static unsigned long g_uTicks = 0;
static float g_uFps = 0;

static Decoder *g_pclDecoders[MAX_DECODERS];

void setup()
{		
	g_pclDecoders[0] = new ServoTurnoutDecoder(
		0,			//flags
		{ 10 },		//pin
		15,			//range
		10,		//ticks
		{ 11 },		//powerPin
		{ 12 }		//frogPin
	);

	g_uLastFrameTime = millis();
}

void loop() 
{		
	auto currentTime = millis();
	int seconds = 0;

	auto ticks = currentTime - g_uLastFrameTime;
	if (ticks == 0)
		return;

	for (int i = 0; i < MAX_DECODERS; ++i)
	{
		g_pclDecoders[i]->Update(ticks);
	}
}
