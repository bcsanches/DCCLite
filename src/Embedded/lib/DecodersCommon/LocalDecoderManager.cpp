// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "LocalDecoderManager.h"

#include "Console.h"
#include "SensorDecoder.h"
#include "ServoTurnoutDecoder.h"

class Decoder;

#ifdef ARDUINO_AVR_MEGA2560
#define MAX_DECODERS 48
#else
#define MAX_DECODERS 16
#endif

static const char LocalDecoderManagerModuleName[] PROGMEM = { "LDecMgr" };
#define MODULE_NAME Console::FlashStr(LocalDecoderManagerModuleName)

static const char FStrNoSlots[] PROGMEM = { "Out of slots" };
#define FSTR_NO_SLOTS Console::FlashStr(FStrNoSlots)

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

static uint8_t g_iNextSlot = 0;


ServoTurnoutDecoder *LocalDecoderManager::CreateServoTurnout(
	uint8_t flags,
	dcclite::PinType_t pin,
	uint8_t range,
	uint8_t ticks,
	dcclite::PinType_t powerPin,
	dcclite::PinType_t frogPin
)
{
	if (g_iNextSlot >= MAX_DECODERS)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_NO_SLOTS);

		return nullptr;
	}

	auto *turnout = new ServoTurnoutDecoder(flags, pin, range, ticks, powerPin, frogPin);

	g_pDecoders[g_iNextSlot++] = turnout;	

	return turnout;
}


SensorDecoder *LocalDecoderManager::CreateSensor(
	uint8_t flags,
	dcclite::PinType_t pin,
	uint8_t activateDelay,
	uint8_t deactivateDelay
)
{
	if (g_iNextSlot >= MAX_DECODERS)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_NO_SLOTS);

		return nullptr;
	}

	auto *sensor = new SensorDecoder(flags, pin, activateDelay, deactivateDelay);

	g_pDecoders[g_iNextSlot++] = sensor;

	return sensor;
}

Decoder *LocalDecoderManager::TryGetDecoder(const uint8_t slot)
{
	assert(slot < MAX_DECODERS);

	return g_pDecoders[slot];
}

bool LocalDecoderManager::Update(const unsigned long ticks)
{
	bool stateChanged = false;

	for (size_t i = 0; i < g_iNextSlot; ++i)
	{		
		stateChanged |= g_pDecoders[i]->Update(ticks);
	}

	return stateChanged;
}

