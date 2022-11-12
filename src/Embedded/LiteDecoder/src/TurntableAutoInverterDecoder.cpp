// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TurntableAutoInverterDecoder.h"

#include <Arduino.h>

#include "Console.h"
#include "DecoderManager.h"
#include "Packet.h"
#include "SensorDecoder.h"
#include "SharedLibDefs.h"
#include "Storage.h"

#define MODULE_NAME F("TurntableAID")

auto constexpr TRACK_TURNOFF_TICKS = 5;

TurntableAutoInverterDecoder::TurntableAutoInverterDecoder(dcclite::Packet& packet) noexcept:
	Decoder::Decoder(packet)	
{
	m_fFlags = packet.Read<uint8_t>() & dcclite::TRTD_ACTIVE;
	
	m_uSensorAIndex = packet.Read<uint8_t>();
	m_uSensorBIndex = packet.Read<uint8_t>();

	dcclite::PinType_t trackPins[4];

	for(int i = 0;i < 4; ++i)
		trackPins[i] = packet.Read<dcclite::PinType_t>();
	
	this->Init(trackPins);

	//Console::SendLogEx(MODULE_NAME, F("Packet"));
}

TurntableAutoInverterDecoder::TurntableAutoInverterDecoder(Storage::EpromStream& stream) noexcept:
	Decoder::Decoder(stream)
{	
	m_uFlagsStorageIndex = stream.GetIndex();

	stream.Get(m_fFlags);

	//Consider only active flag
	m_fFlags = m_fFlags & dcclite::TRTD_ACTIVE;

	stream.Get(m_uSensorAIndex);
	stream.Get(m_uSensorBIndex);
	
	dcclite::PinType_t trackPins[4];

	for (int i = 0; i < 4; ++i)
		stream.Get(trackPins[i]);	

	this->Init(trackPins);

	//Console::SendLogEx(MODULE_NAME, F("Stream"));
}

void TurntableAutoInverterDecoder::SaveConfig(Storage::EpromStream& stream) noexcept
{
	Decoder::SaveConfig(stream);

	m_uFlagsStorageIndex = stream.GetIndex();

	stream.Put(m_fFlags);
	stream.Put(m_uSensorAIndex);
	stream.Put(m_uSensorBIndex);

	for(int i = 0; i < 4; ++i)
		stream.Put(m_arTrackPins[i].Raw());	
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

void TurntableAutoInverterDecoder::TurnOnTrackPower() noexcept
{
	if (m_fFlags & dcclite::TRTD_ACTIVE)
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

void TurntableAutoInverterDecoder::Init(const dcclite::PinType_t trackPins[4]) noexcept
{
	using namespace dcclite;	

	for (int i = 0; i < 4; ++i)
	{
		m_arTrackPins[i].Attach(trackPins[i], Pin::MODE_OUTPUT);
		m_arTrackPins[i].DigitalWrite(Pin::VLOW);
	}		

	//Console::SendLogEx(MODULE_NAME, F("Init"));
	//DCCLITE_LOG_MODULE_LN(F("init"));	

	//wait 5 seconds, should be enough to load everything and start pooling sensors
	m_uWaitingTrackTurnOff = 5000;
}

bool TurntableAutoInverterDecoder::AcceptServerState(dcclite::DecoderStates state) noexcept
{
	if (state == dcclite::DecoderStates::ACTIVE)
	{
		m_fFlags |= dcclite::TRTD_REMOTE_ACTIVE;

		//Console::SendLogEx("[TurntableAutoInverterDecoder]", "remote", ' ', "ACTIVE");
	}
	else
	{
		m_fFlags &= ~dcclite::TRTD_REMOTE_ACTIVE;

		//Console::SendLogEx("[TurntableAutoInverterDecoder]", "remote", ' ', "INACTIVE");
	}

	//Console::SendLogEx("[TurntableAutoInverterDecoder]", "SYNC", ' ', this->IsSyncRequired());
	return this->IsSyncRequired();
}

bool TurntableAutoInverterDecoder::Update(const unsigned long ticks) noexcept
{
	if (m_uWaitingTrackTurnOff > 0)
	{
		//waiting track turn off?
		if (m_uWaitingTrackTurnOff > ticks)
			return false;

		//
		//track turnoff timeout, so do it
		//Console::SendLogEx(MODULE_NAME, F("Update cooldown finished"));
		this->TurnOnTrackPower();
		m_uWaitingTrackTurnOff = 0;

		return false;
	}
	
	if (m_fFlags & dcclite::TRTD_ACTIVE)
	{
		bool sensorState;

		if(!DecoderManager::GetDecoderActiveStatus(m_uSensorBIndex, sensorState))
			return false;		

		if (!sensorState)
			return false;
		
		//Console::SendLogEx(MODULE_NAME, F("Update m_pclSensorB"));
		DCCLITE_LOG_MODULE_LN(F("Update m_pclSensorB"));
		TurnTrackOff(m_arTrackPins + 2);

		m_fFlags = m_fFlags & ~dcclite::TRTD_ACTIVE;		
	}
	else
	{
		bool sensorState;

		if(!DecoderManager::GetDecoderActiveStatus(m_uSensorAIndex, sensorState))
			return false;		

		if (!sensorState)
			return false;

		//Console::SendLogEx(MODULE_NAME, F("Update m_pclSensorA"));
		DCCLITE_LOG_MODULE_LN(F("Update m_pclSensorA"));

		TurnTrackOff(m_arTrackPins);		

		m_fFlags = m_fFlags | dcclite::TRTD_ACTIVE;		
	}

	if(m_uFlagsStorageIndex)
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);

	//Console::SendLogEx(MODULE_NAME, F("Update waiting"));
	DCCLITE_LOG_MODULE_LN(F("Update waiting"));

	m_uWaitingTrackTurnOff = ticks + TRACK_TURNOFF_TICKS;
	return true;		
}
