// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SensorDecoder.h"

#include <Arduino.h>

#include <dcclite_shared/Packet.h>
#include <dcclite_shared/SharedLibDefs.h>

#include "Console.h"
#include "Storage.h"

#define MODULE_NAME F("SensorDecoder")

//#define DCCLITE_DBG

SensorDecoder::SensorDecoder(uint8_t flags, dcclite::PinType_t pin, uint16_t activateDelay, uint16_t deactivateDelay, uint16_t startDelay) noexcept:	
	m_uActivateDelay{ activateDelay },
	m_uDeactivateDelay{deactivateDelay},
	m_uStartDelay{ startDelay },
	m_fFlags{ flags }
{
#ifdef DCCLITE_DBG
	Console::SendLogEx("[SENSOR_DECODER]", "constructor");
#endif

	this->Init(pin);
}

SensorDecoder::SensorDecoder(dcclite::Packet &packet) noexcept:
	Decoder::Decoder(packet)
{
	const auto pin = packet.Read<dcclite::PinType_t>();
	
	//only read pull up and inverted flag, the others are internal
	m_fFlags = packet.Read<uint8_t>() & (dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED);
	m_uActivateDelay = packet.Read<uint16_t>();
	m_uDeactivateDelay = packet.Read<uint16_t>();
	m_uStartDelay = packet.Read<uint16_t>();

	using namespace dcclite;

	this->Init(pin);
}

SensorDecoder::SensorDecoder(Storage::EpromStream &stream) noexcept:
	Decoder::Decoder(stream)
{
	dcclite::PinType_t pin;

	stream.Get(pin);	
	stream.Get(m_fFlags);
	stream.Get(m_uActivateDelay);
	stream.Get(m_uDeactivateDelay);
	stream.Get(m_uStartDelay);

	//only read pull up and inverted flag, the others are internal
	m_fFlags = m_fFlags & (dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED);

	this->Init(pin);
}


void SensorDecoder::SaveConfig(Storage::EpromStream& stream) noexcept
{
	Decoder::SaveConfig(stream);

	stream.Put(m_clPin.Raw());	
	stream.Put(m_fFlags);
	stream.Put(m_uActivateDelay);
	stream.Put(m_uDeactivateDelay);
	stream.Put(m_uStartDelay);
}

void SensorDecoder::Init(const dcclite::PinType_t pin) noexcept
{
	using namespace dcclite;	

	m_clPin.Attach(pin, (m_fFlags & SNRD_PULL_UP) ? Pin::MODE_INPUT_PULLUP : Pin::MODE_INPUT);

	if (m_uStartDelay)
	{
		m_fFlags |= SNRD_DELAY;
		m_uCoolDownTicks = millis() + m_uStartDelay;

		DCCLITE_LOG_MODULE_LN(F("Start") << ' ' << F("delay") << ' ' << m_uStartDelay);
	}
}

bool SensorDecoder::AcceptServerState(dcclite::DecoderStates state, const unsigned long ticks) noexcept
{
	if (state == dcclite::DecoderStates::ACTIVE)
	{
		m_fFlags |= dcclite::SNRD_REMOTE_ACTIVE;

		//Console::SendLogEx("[SENSOR_DECODER]", "remote", ' ', "ACTIVE");
	}
	else
	{
		m_fFlags &= ~dcclite::SNRD_REMOTE_ACTIVE;

		//Console::SendLogEx("[SENSOR_DECODER]", "remote", ' ', "INACTIVE");
	}

	//Console::SendLogEx("[SENSOR_DECODER]", "SYNC", ' ', this->IsSyncRequired());
	return this->IsSyncRequired();
}

bool SensorDecoder::Update(const unsigned long ticks) noexcept
{
	const bool coolDown = m_fFlags & dcclite::SNRD_COOLDOWN;	
	const bool delay = m_fFlags & dcclite::SNRD_DELAY;

	//if on cooldown state and not finished yet
	if ((coolDown || delay) && (ticks < m_uCoolDownTicks))
	{
		//Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "COOLDOWN", ' ', "ABORT");

		//wait...
		return false;
	}		

	//disable cooldown and delay anyway
	m_fFlags &= ~(dcclite::SNRD_COOLDOWN | dcclite::SNRD_DELAY);

	bool state = m_clPin.DigitalRead() == Pin::VHIGH;		
	state = (m_fFlags & dcclite::SNRD_INVERTED) ? !state : state;

	bool previousState = m_fFlags & dcclite::SNRD_ACTIVE;

	//no state change?
	if (state == previousState)
	{		
		return false;
	}

#ifdef DCCLITE_DBG
	Console::SendLogEx("[SENSOR_DECODER]", "noise");
#endif

	if((!coolDown) && (!delay))
	{
		//are we starting a state change?
		m_fFlags |= dcclite::SNRD_COOLDOWN;

		m_uCoolDownTicks = ticks + CFG_COOLDOWN_TIMEOUT_TICKS;

#ifdef DCCLITE_DBG
		Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "COOLDOWN");
#endif

		return false;
	}
		
	if (coolDown)
	{	
		//do we have to wait for a state change?
		const long delayTicks = ((m_fFlags & dcclite::SNRD_ACTIVE) ? m_uDeactivateDelay : m_uActivateDelay);
		if(delayTicks)
		{
			//ok, prepare for wait and get out
			m_uCoolDownTicks = ticks + delayTicks;
			m_fFlags |= dcclite::SNRD_DELAY;

#ifdef DCCLITE_DBG
			Console::SendLogEx("[SENSOR_DECODER]", "DELAY", ' ', (int)delayTicks);
#endif

			return false;
		}
	}		

	//we finished delay and cooldown, register the state change
	if(m_fFlags & dcclite::SNRD_ACTIVE)
		m_fFlags &= ~dcclite::SNRD_ACTIVE;
	else
		m_fFlags |= dcclite::SNRD_ACTIVE;	

#ifdef DCCLITE_DBG
	Console::SendLogEx("[SENSOR_DECODER]", "STAGE", ' ', "CHANGE");
#endif

	return true;		
}
