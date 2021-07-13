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

#define MAX_BUTTONS 32

static const char LocalDecoderManagerModuleName[] PROGMEM = { "LDecMgr" };
#define MODULE_NAME Console::FlashStr(LocalDecoderManagerModuleName)

static const char FStrNoSlots[] PROGMEM = { "Out of slots" };
#define FSTR_NO_SLOTS Console::FlashStr(FStrNoSlots)

static const char FStrNoButtons[] PROGMEM = { "Out of buttons" };
#define FSTR_NO_BUTTONS Console::FlashStr(FStrNoButtons)

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

static uint8_t g_iNextSlot = 0;

//
//
// Button
//
//

class Button
{
	public:		
		Button(SensorDecoder &sensor, ServoTurnoutDecoder &target, LocalDecoderManager::ButtonActions action);

		void Update();

	private:
		SensorDecoder &m_rclSensor;
		ServoTurnoutDecoder &m_rclTarget;

		LocalDecoderManager::ButtonActions m_kAction;

		bool m_fButtonPressed = false;
};

Button::Button(SensorDecoder &sensor, ServoTurnoutDecoder &target, LocalDecoderManager::ButtonActions action) :
	m_rclSensor(sensor),
	m_rclTarget(target),
	m_kAction(action)
{
	//empty
}

void Button::Update()
{
	const bool buttonPreviousState = m_fButtonPressed;
	m_fButtonPressed = m_rclSensor.IsActive();

	//sensor in the same state?
	if (buttonPreviousState == m_fButtonPressed)
		return;

	//Sensor was pressed, now released... ok nothing to do
	if (buttonPreviousState)
	{
		Console::Send("Button released");

		return;
	}

	Console::Send("Button pressed");

	//
	//if we reached here, button was pressed on this frame, so call the target....
	if (m_kAction == LocalDecoderManager::kTHROW)
		m_rclTarget.AcceptServerState(dcclite::DecoderStates::ACTIVE);
	else if (m_kAction == LocalDecoderManager::kCLOSE)
		m_rclTarget.AcceptServerState(dcclite::DecoderStates::INACTIVE);
	else
	{
		auto state = m_rclTarget.GetDecoderState();
		state = !state;

		m_rclTarget.AcceptServerState(state);
	}
}

static Button *g_pButtons[MAX_BUTTONS] = { 0 };

static uint8_t g_iNextButton = 0;

//
//
// LocalDecoderManager
//
//

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

void LocalDecoderManager::CreateButton(SensorDecoder &sensor, ServoTurnoutDecoder &target, ButtonActions actions)
{
	if (g_iNextButton >= MAX_BUTTONS)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_NO_BUTTONS);

		return;
	}

	g_pButtons[g_iNextButton++] = new Button(sensor, target, actions);
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

	for (auto i = 0; i < g_iNextButton; ++i)
	{
		g_pButtons[i]->Update();
	}

	return stateChanged;
}

