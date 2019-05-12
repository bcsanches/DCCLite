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
#include <EmbeddedLibDefs.h>
#include <Packet.h>

#include "Config.h"
#include "Console.h"
#include "Storage.h"

SensorDecoder::SensorDecoder(dcclite::Packet& packet) :
	Decoder::Decoder(packet)
{
	m_tPin = packet.Read<Pin_t>();

	//only read pull up flag, the others are internal
	m_fFlags = packet.Read<uint8_t>() & dcclite::SNRD_PULL_UP;

	using namespace dcclite;

	this->Init();
}

SensorDecoder::SensorDecoder(EpromStream& stream) :
	Decoder::Decoder(stream)
{
	stream.Get(m_tPin);	
	stream.Get(m_fFlags);

	this->Init();
}


void SensorDecoder::SaveConfig(EpromStream& stream)
{
	Decoder::SaveConfig(stream);

	stream.Put(m_tPin);	
	stream.Put(m_fFlags);
}

void SensorDecoder::Init()
{
	using namespace dcclite;	

	pinMode(m_tPin, m_fFlags & SNRD_PULL_UP ? INPUT : INPUT_PULLUP);
}

bool SensorDecoder::AcceptServerState(dcclite::DecoderStates state)
{
	if (state == dcclite::DecoderStates::ACTIVE)
	{
		m_fFlags |= dcclite::SNRD_REMOTE_ACTIVE;

		Console::SendLogEx("[SENSOR_DECODER]", "remote", ' ', "ACTIVE");
	}
	else
	{
		m_fFlags &= ~dcclite::SNRD_REMOTE_ACTIVE;

		Console::SendLogEx("[SENSOR_DECODER]", "remote", ' ', "INACTIVE");
	}

	return this->IsSyncRequired();
}

bool SensorDecoder::Update(const unsigned long ticks)
{
	bool coolDown = m_fFlags & dcclite::SNRD_COOLDOWN;	

	//if on cooldown state and not finished yet
	if (coolDown && (ticks < m_uCoolDownTicks))
	{
		Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "COOLDOWN", ' ', "ABORT");

		//wait...
		return false;
	}

	//disable cooldown anyway
	m_fFlags &= ~dcclite::SNRD_COOLDOWN;

	int state = digitalRead(m_tPin);

	int previousState = m_fFlags & dcclite::SNRD_ACTIVE ? HIGH : LOW;

	//no state change?
	if (state == previousState)
	{		
		return false;
	}
	
	if (coolDown)
	{		
		if (state == LOW)
		{
			m_fFlags |= dcclite::SNRD_ACTIVE;

			Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "ACTIVATED");
		}
		else
		{
			m_fFlags &= ~dcclite::SNRD_ACTIVE;

			Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "INACTIVATED");

		}

		//state changed
		return true;
	}
	else
	{
		//are we starting a state change?
		m_fFlags |= dcclite::SNRD_COOLDOWN;

		m_uCoolDownTicks = ticks + Config::g_cfgCoolDownTimeoutTicks;

		Console::SendLogEx("[SENSOR_DECODER]", "LOCAL", ' ', "COOLDOWN");

		return false;
	}	
}
