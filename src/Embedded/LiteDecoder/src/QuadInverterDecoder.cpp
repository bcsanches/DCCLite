// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "QuadInverterDecoder.h"

#include <Arduino.h>

#include <dcclite_shared/Packet.h>

#include "Console.h"
#include "DecoderManager.h"
#include "SensorDecoder.h"
#include "Storage.h"

#define MODULE_NAME F("QuadAID")

QuadInverterDecoder::QuadInverterDecoder(dcclite::Packet& packet) noexcept:
	Decoder::Decoder(packet)	
{
	//consider only configurable flags
	m_fFlags = packet.Read<uint8_t>() & (dcclite::QUAD_IGNORE_SAVED_STATE | dcclite::QUAD_ACTIVATE_ON_POWER_UP);

	m_u8FlipInterval = packet.Read<uint8_t>();

	//DCCLITE_LOG_MODULE_LN(F("QuadInverterDecoder packet flags") << (int) m_fFlags);
		
	dcclite::PinType_t trackPins[4];

	for(int i = 0;i < 4; ++i)
		trackPins[i] = packet.Read<dcclite::PinType_t>();
	
	this->Init(trackPins);

	//Console::SendLogEx(MODULE_NAME, F("Packet"));
}

QuadInverterDecoder::QuadInverterDecoder(Storage::EpromStream& stream) noexcept:
	Decoder::Decoder(stream)
{	
	m_uFlagsStorageIndex = stream.GetIndex();

	stream.Get(m_fFlags);	
	stream.Get(m_u8FlipInterval);

	//DCCLITE_LOG_MODULE_LN(F("QuadInverterDecoder eprom flags") << (int) m_fFlags);
	
	dcclite::PinType_t trackPins[4];

	for (int i = 0; i < 4; ++i)
		stream.Get(trackPins[i]);	

	this->Init(trackPins);

	//Console::SendLogEx(MODULE_NAME, F("Stream"));
}

void QuadInverterDecoder::SaveConfig(Storage::EpromStream& stream) noexcept
{
	Decoder::SaveConfig(stream);

	m_uFlagsStorageIndex = stream.GetIndex();

	stream.Put(m_fFlags);	
	stream.Put(m_u8FlipInterval);
	//DCCLITE_LOG_MODULE_LN(F("QuadInverterDecoder SaveConfig flags") << (int) m_fFlags);

	for(int i = 0; i < 4; ++i)
		stream.Put(m_arTrackPins[i].Raw());	
}

void QuadInverterDecoder::Init(const dcclite::PinType_t trackPins[4]) noexcept
{
	using namespace dcclite;	
	
	for (int i = 0; i < 4; ++i)
	{
		m_arTrackPins[i].Attach(trackPins[i], Pin::MODE_OUTPUT);
		m_arTrackPins[i].DigitalWrite(Pin::VLOW);
	}		

	if (m_fFlags & dcclite::QUAD_IGNORE_SAVED_STATE)
	{
		if (m_fFlags & dcclite::QUAD_ACTIVATE_ON_POWER_UP)
		{
			m_fFlags |= QUAD_ACTIVE;
		}
		else
		{
			m_fFlags &= ~QUAD_ACTIVE;
		}
	}

	//Console::SendLogEx(MODULE_NAME, F("Init"));
	//DCCLITE_LOG_MODULE_LN(F("init"));	

	//wait 5 seconds, should be enough to load everything
	m_uWaitingTrackTurnOff = 5000;
}

inline void TurnTrackOn(Pin track[2])
{
	track[0].DigitalWrite(Pin::VHIGH);
	track[1].DigitalWrite(Pin::VHIGH);
}

inline void TurnTrackOff(Pin track[2])
{
	track[0].DigitalWrite(Pin::VLOW);
	track[1].DigitalWrite(Pin::VLOW);
}

void QuadInverterDecoder::TurnOnTrackPower() noexcept
{
	if (m_fFlags & dcclite::QUAD_ACTIVE)
	{
		//Console::SendLogEx(MODULE_NAME, F("TurnOnTrackPower TrackB"));
		DCCLITE_LOG_MODULE_LN(F("TurnOnTrackPower Track") << 'B');

		TurnTrackOn(m_arTrackPins + 2);
	}
	else
	{
		//Console::SendLogEx(MODULE_NAME, F("TurnOnTrackPower TrackA"));
		DCCLITE_LOG_MODULE_LN(F("TurnOnTrackPower Track") << 'A');

		TurnTrackOn(m_arTrackPins);
	}
}

bool QuadInverterDecoder::AcceptServerState(dcclite::DecoderStates state, const unsigned long time) noexcept
{
	const bool currentState = m_fFlags & dcclite::QUAD_ACTIVE;
	const bool remoteState = state == dcclite::DecoderStates::ACTIVE;

	if (currentState == remoteState)
	{
		//no state change
		return false;
	}

	if (remoteState)
	{
		TurnTrackOff(m_arTrackPins);

		m_fFlags |= dcclite::QUAD_ACTIVE;
	}
	else
	{
		TurnTrackOff(m_arTrackPins + 2);

		m_fFlags &= ~dcclite::QUAD_ACTIVE;
	}

	m_uWaitingTrackTurnOff = time + m_u8FlipInterval;

	if (m_uFlagsStorageIndex)
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);
	
	return true;
}

bool QuadInverterDecoder::Update(const unsigned long time) noexcept
{		
	if ((m_uWaitingTrackTurnOff > 0) && (m_uWaitingTrackTurnOff <= time))
	{
		//
		//track turnoff timeout, so do it
		//Console::SendLogEx(MODULE_NAME, F("Update cooldown finished"));
		this->TurnOnTrackPower();
		m_uWaitingTrackTurnOff = 0;
	}
				
	return false;
}
