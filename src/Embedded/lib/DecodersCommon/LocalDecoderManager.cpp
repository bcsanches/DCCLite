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
#include "Storage.h"
#include "SensorDecoder.h"
#include "ServoTurnoutDecoder.h"

class Decoder;

#ifdef ARDUINO_AVR_MEGA2560
#define MAX_DECODERS 48
#else
constexpr auto MAX_DECODERS = 16;
#endif

constexpr auto MAX_BUTTONS = 32;

#define MODULE_NAME F("LocalDecoderManager")
#define FSTR_NO_SLOTS F("Out of slots")

#define FSTR_NO_BUTTONS F("Out of buttons")

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

static uint8_t g_iNextSlot = 0;

static const char *g_psDate = "";
static const char *g_psTime = "";

static Storage::EpromStream *g_pLoadStream = nullptr;

#define EPROM_HEADER_SIZE 32

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
		Console::SendLn("Button released");

		return;
	}	

	//
	//if we reached here, button was pressed on this frame, so call the target....
	if (m_kAction == LocalDecoderManager::kTHROW)
	{	
		Console::Send("Button pressed - Throw");
		m_rclTarget.AcceptServerState(dcclite::DecoderStates::ACTIVE);
	}
	else if (m_kAction == LocalDecoderManager::kCLOSE)
	{
		Console::Send("Button pressed - Close");
		m_rclTarget.AcceptServerState(dcclite::DecoderStates::INACTIVE);
	}		
	else
	{
		auto state = m_rclTarget.GetDecoderState();
		state = !state;

		Console::Send("Button pressed - Toggle");

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

	auto *turnout = g_pLoadStream ? new ServoTurnoutDecoder(*g_pLoadStream) : new ServoTurnoutDecoder(flags, pin, range, ticks, powerPin, frogPin);

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

	auto *sensor = g_pLoadStream ? new SensorDecoder(*g_pLoadStream) : new SensorDecoder(flags, pin, activateDelay, deactivateDelay);

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

	Console::SendLogEx(MODULE_NAME, "Added button", ' ', g_iNextButton);
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

struct Header
{
	char m_u8Buffer[EPROM_HEADER_SIZE];
};

static void InitHeader(Header &header)
{
	memset(&header, 0, sizeof(header));
	strncpy(header.m_u8Buffer, g_psDate, 16);
	strncpy(header.m_u8Buffer + 16, g_psTime, 16);
}

void LocalDecoderManager::Init(const char *time, const char *date)
{
	assert(time);
	assert(date);

	g_psTime = time;
	g_psDate = date;

	Header header;
	
	InitHeader(header);	

	g_pLoadStream = new Storage::EpromStream{ 0 };	

	Header storageData;
	g_pLoadStream->GetRaw(reinterpret_cast<uint8_t *>(storageData.m_u8Buffer), sizeof(storageData.m_u8Buffer));
	
	if (memcmp(storageData.m_u8Buffer, header.m_u8Buffer, sizeof(storageData.m_u8Buffer)) == 0)
	{
		Console::SendLogEx(MODULE_NAME, "Found valid eprom data");		
	}
	else
	{
		Console::SendLogEx(MODULE_NAME, "NO eprom data");
		Console::SendLogEx(MODULE_NAME, storageData.m_u8Buffer, storageData.m_u8Buffer+16);

		delete g_pLoadStream;
		g_pLoadStream = nullptr;
	}
	
}

void LocalDecoderManager::PostInit()
{
	if (g_pLoadStream)
	{
		//nothing to do
		delete g_pLoadStream;
		g_pLoadStream = nullptr;

		Console::SendLogEx(MODULE_NAME, "Post Init OK");
		return;
	}

	Console::SendLogEx(MODULE_NAME, "Initializing eprom");

	Header header;	
	InitHeader(header);	

	Storage::EpromStream stream{ 0 };

	stream.PutRawData(reinterpret_cast<uint8_t *>(header.m_u8Buffer), sizeof(header));
	Console::SendLogEx(MODULE_NAME, header.m_u8Buffer, header.m_u8Buffer+16);	

	for (int i = 0; i < g_iNextSlot; ++i)
	{
		g_pDecoders[i]->SaveConfig(stream);
	}

	Console::SendLogEx(MODULE_NAME, "Saved ", g_iNextSlot, " decoders ", (int)stream.GetIndex());	
}
