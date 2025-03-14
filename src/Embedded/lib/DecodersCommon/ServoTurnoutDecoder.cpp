// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ServoTurnoutDecoder.h"

#include <dcclite_shared/Packet.h>

#include "Console.h"
#include "Storage.h"

constexpr auto POWER_OFF_TICKS = 500;
constexpr auto POWER_WAIT_TICKS = 300;

#if 1
#define SERVO_WRITE(servo, data) servo.write(data)
#else
#define SERVO_WRITE(servo, data) Console::SendLogEx("SERVO pos ", data, " line ", __LINE__); servo.write(data)
#endif


ServoTurnoutDecoder::ServoTurnoutDecoder(dcclite::Packet& packet) :
	Decoder::Decoder(packet),
	m_clPin{packet.Read<dcclite::PinType_t>()}
{	
	m_fFlags = packet.Read<uint8_t>();

	this->SetState(States::CLOSED);	

	auto powerPin = packet.Read<dcclite::PinType_t>();	
	auto frogPin = packet.Read<dcclite::PinType_t>();
	
	m_uStartPos = packet.Read<uint8_t>();
	m_uEndPos = packet.Read<uint8_t>();
	m_uTicks = packet.Read<uint8_t>();

	this->Init(powerPin, frogPin, false);
}

ServoTurnoutDecoder::ServoTurnoutDecoder(Storage::EpromStream& stream) :
	Decoder::Decoder(stream)
{	
	dcclite::PinType_t pin;
	stream.Get(pin);

	m_clPin = dcclite::BasicPin{ pin };

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);	

	dcclite::PinType_t powerPin;
	stream.Get(powerPin);

	dcclite::PinType_t frogPin;
	stream.Get(frogPin);	

	stream.Get(m_uStartPos);
	stream.Get(m_uEndPos);
	stream.Get(m_uTicks);	

	this->Init(powerPin, frogPin, false);
}

ServoTurnoutDecoder::ServoTurnoutDecoder(uint8_t flags, dcclite::PinType_t pin, uint8_t range, uint8_t ticks, dcclite::PinType_t powerPin, dcclite::PinType_t frogPin):		
	m_clPin{ pin },
	m_fFlags{ flags },	
	m_uStartPos{ 0 },
	m_uEndPos{ range },
	m_uTicks{ ticks }
{
	this->Init(powerPin, frogPin, true);
}

ServoTurnoutDecoder::~ServoTurnoutDecoder()
{
	m_clServo.detach();

	this->TurnOffPower();
}


void ServoTurnoutDecoder::SaveConfig(Storage::EpromStream& stream)
{
	Decoder::SaveConfig(stream);

	stream.Put(m_clPin.Raw());

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Put(m_fFlags);

	stream.Put(m_clPowerPin.Raw());
	stream.Put(m_clFrogPin.Raw());
	
	stream.Put(m_uStartPos);
	stream.Put(m_uEndPos);
	stream.Put(m_uTicks);
}

void ServoTurnoutDecoder::TurnOnPower(const unsigned long time) noexcept
{
	SERVO_WRITE(m_clServo, m_uServoPos);
	m_clServo.attach(m_clPin.Raw());	

	if (m_clPowerPin)		
	{
		m_clPowerPin.DigitalWrite((m_fFlags & dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_POWER) ? Pin::VHIGH : Pin::VLOW);
	}	
		
	m_fFlags |= dcclite::SRVT_POWER_ON;
	m_uNextThink = time + POWER_WAIT_TICKS;
}

void ServoTurnoutDecoder::TurnOffPower() noexcept
{
	if (m_clPowerPin)		
	{
		m_clPowerPin.DigitalWrite((m_fFlags & dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_POWER) ? Pin::VLOW : Pin::VHIGH);		
	}

	m_clServo.detach();
	
	m_fFlags &= ~dcclite::SRVT_POWER_ON;

	//Console::SendLogEx("SERVO", "POWEROFF");
}

void ServoTurnoutDecoder::Init(const dcclite::PinType_t powerPin, const dcclite::PinType_t frogPin, bool checkInversion) noexcept
{
	using namespace dcclite;	
	
	if(!dcclite::IsPinNull(powerPin))
	{
		m_clPowerPin.Attach(powerPin, Pin::MODE_OUTPUT);
		
		this->TurnOffPower();
	}
	
	if(!dcclite::IsPinNull(frogPin))
	{
		m_clFrogPin.Attach(frogPin, Pin::MODE_OUTPUT);		
	}
		
	States desiredState = this->GetStateGoal();	

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	if (m_fFlags & SRVT_IGNORE_SAVED_STATE)
	{		
		desiredState = (m_fFlags & SRVT_ACTIVATE_ON_POWER_UP) ? States::THROWN : States::CLOSED;		

		checkInversion = true; 		
	}		

	if ((checkInversion) && (m_fFlags & SRVT_INVERTED_OPERATION))
	{
		desiredState = desiredState == States::THROWN ? States::CLOSED : States::THROWN;
	}
	
	if (desiredState == States::THROWN)
	{
		m_uServoPos = m_uEndPos;
		this->OperateThrown(millis());
	}
	else
	{
		m_uServoPos = m_uStartPos;
		this->OperateClose(millis());
	}		

#if 0
	Console::SendLogEx("[ServoTurnout]", ' ', "PIN: ", m_tPin);
	Console::SendLogEx("[ServoTurnout]", ' ', "IGNORE_SAVE: ", m_fFlags & SRVT_IGNORE_SAVED_STATE);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVATE_ON_POWERUP: ", m_fFlags & SRVT_ACTIVATE_ON_POWER_UP);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVE: ", m_fFlags & SRVT_ACTIVE);
	Console::SendLogEx("[ServoTurnout]", ' ', "INVERTED: ", m_fFlags & SRVT_INVERTED_OPERATION);
#endif	
}

dcclite::DecoderStates ServoTurnoutDecoder::State2DecoderState() const noexcept
{
	auto state = this->GetState();

	if (state == States::CLOSING)
	{
		state = States::THROWN;
	}
	else if (state == States::THROWNING)
	{
		state = States::CLOSED;
	}

	bool active = state == States::THROWN;
	active = (m_fFlags & dcclite::SRVT_INVERTED_OPERATION) ? !active : active;

	return active ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;
}

ServoTurnoutDecoder::States ServoTurnoutDecoder::DecoderState2State(dcclite::DecoderStates state) const noexcept
{
	bool activate = state == dcclite::DecoderStates::ACTIVE;
	activate = (m_fFlags & dcclite::SRVT_INVERTED_OPERATION) ? !activate : activate;

	return activate ? States::THROWN : States::CLOSED;
}

bool ServoTurnoutDecoder::AcceptServerState(const dcclite::DecoderStates decoderState, const unsigned long time)
{
	using namespace dcclite;

	auto requestedState = this->DecoderState2State(decoderState);

	//no state change?
	if (requestedState == this->GetStateGoal())
	{
#if 0
		Console::SendLogEx("[OutputDecoder]", "got state, but ignored (same)");
#endif

		return false;
	}

	if (requestedState == States::THROWN)
		this->OperateThrown(time);
	else
		this->OperateClose(time);

	return true;
}

bool ServoTurnoutDecoder::StateUpdate(const uint8_t desiredPosition, const States desiredState, const int moveDirection, const unsigned long ticks) noexcept
{
	//first check desired position, we may reach it right on initial state
	if (m_uServoPos == desiredPosition)
	{
		//make sure we are in position, on init state we may not, so we write it again
		SERVO_WRITE(m_clServo, m_uServoPos); 

		m_uNextThink = ticks + POWER_OFF_TICKS;

		this->SetState(desiredState);

		if (m_clFrogPin)
		{
			bool activateFrog = desiredState == States::THROWN;
			activateFrog = (m_fFlags & dcclite::SRVT_INVERTED_FROG) ? !activateFrog : activateFrog;

			m_clFrogPin.DigitalWrite(activateFrog ? Pin::VHIGH : Pin::VLOW);
		}

		//Store current state on eprom, so we can reload.
		if ((m_uFlagsStorageIndex) && (!(m_fFlags & dcclite::SRVT_IGNORE_SAVED_STATE)))
		{
			//Console::SendLogEx("[SERVO]", "updated ", m_uFlagsStorageIndex);
			Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);
		}			

		//Console::SendLogEx("[SERVO]", "finished", m_uServoPos);

		//state changed
		return true;
	}
	else if (ticks >= m_uNextThink)
	{
		m_uServoPos += moveDirection;

		m_uNextThink = ticks + m_uTicks;
		SERVO_WRITE(m_clServo, m_uServoPos);

		//Console::SendLogEx("[SERVO]", "m_uServoPos", m_uServoPos);
	}	

	return false;
}

bool ServoTurnoutDecoder::Update(const unsigned long ticks)
{
	using namespace dcclite;

	Decoder::Update(ticks);

	const auto state = this->GetState();

	if ((state == States::CLOSED) || (state == States::THROWN))
	{
		if ((m_fFlags & SRVT_POWER_ON) && (m_uNextThink <= ticks))
		{
			this->TurnOffPower();
		}

		return false;
	}
	else if (state == States::CLOSING)
	{
		return this->StateUpdate(m_uStartPos, States::CLOSED, -1, ticks);
	}
	else
	{
		return this->StateUpdate(m_uEndPos, States::THROWN, 1, ticks);
	}		
}

ServoTurnoutDecoder::States ServoTurnoutDecoder::GetState() const noexcept
{
	uint8_t value = (m_fFlags & dcclite::SRVT_STATE_BITS);

	return static_cast<States>(value);
}

void ServoTurnoutDecoder::SetState(const States newState) noexcept
{
	uint8_t bits = static_cast<uint8_t>(newState) & dcclite::SRVT_STATE_BITS;

	m_fFlags &= ~dcclite::SRVT_STATE_BITS;
	m_fFlags |= bits;	
}

void ServoTurnoutDecoder::OperateThrown(const unsigned long time) noexcept
{
	this->TurnOnPower(time);

	this->SetState(States::THROWNING);
}

void ServoTurnoutDecoder::OperateClose(const unsigned long time) noexcept
{
	this->TurnOnPower(time);

	this->SetState(States::CLOSING);
}

void ServoTurnoutDecoder::PM_EnableProgMode() noexcept
{	
	this->SetState(States::THROWN);
	this->TurnOnPower(millis());
}

void ServoTurnoutDecoder::PM_DisableProgMode() noexcept
{
	//go back to reset state
	this->OperateClose(millis());
}
