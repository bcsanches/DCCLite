// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Blinker.h"

#include <assert.h>
#include <Arduino.h>

constexpr auto LOCAL_LED = 13;

constexpr auto SLOW_INTERVAL = 1000;
constexpr auto FAST_INTERVAL = 100;

static unsigned long g_iNextThink = 0;
static Blinker::State g_kState = Blinker::State::OFF;

static uint8_t g_iPulse = 0;
static unsigned long g_iPulseThink = 0;

static bool g_fVoltage = false;

constexpr auto MAX_ANIMATIONS = 2;

void Blinker::Init()
{
	//initialize digital pin 13 as an output.
    pinMode(LOCAL_LED, OUTPUT);
	digitalWrite(LOCAL_LED, LOW);
}

void Blinker::SetState(State state)
{
	if (state == g_kState)
		return;

	g_kState = state;
	g_iPulse = 0;

	switch (g_kState)
	{
		case State::ON:			
			digitalWrite(LOCAL_LED, HIGH);				
			break;

		case State::OFF:			
			digitalWrite(LOCAL_LED, LOW);			
			break;

		case State::FAST_FLASH:
		case State::SLOW_FLASH:
			digitalWrite(LOCAL_LED, LOW);
			g_iNextThink = 0;
			break;
	}	
}

void Blinker::Pulse(unsigned char count)
{
	//ignore pulse when flashing
	if ((g_kState != State::OFF) && (g_kState != State::ON))
		return;

	g_iPulse = count * 2;
	g_iPulseThink = 0;

	g_fVoltage = (g_kState == State::ON);
}

void Blinker::Update(unsigned long ticks)
{
	switch (g_kState)
	{
		case State::OFF:
		case State::ON:
			if ((g_iPulse) && (ticks >= g_iPulseThink))
			{
				--g_iPulse;

				g_fVoltage = !g_fVoltage;
				digitalWrite(LOCAL_LED, g_fVoltage ? HIGH : LOW);

				g_iPulseThink = ticks + (FAST_INTERVAL >> 1);
			}
			break;

		case State::SLOW_FLASH:
		case State::FAST_FLASH:
			if (ticks < g_iNextThink)
				return;

			g_fVoltage = !g_fVoltage;
			digitalWrite(LOCAL_LED, g_fVoltage ? HIGH : LOW);

			g_iNextThink = ticks + (g_kState == State::SLOW_FLASH ? SLOW_INTERVAL : FAST_INTERVAL);
			break;	
    }
}
