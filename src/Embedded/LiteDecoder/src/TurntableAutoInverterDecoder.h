// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Decoder.h"
#include "Pin.h"

class SensorDecoder;

class TurntableAutoInverterDecoder : public Decoder
{
	private:		
		uint8_t			m_uSensorAIndex;
		uint8_t			m_uSensorBIndex;

		//0 and 1, trackA | 2 and 3, track B
		Pin				m_arTrackPins[4];				

		unsigned long	m_uWaitingTrackTurnOff = 0;

		uint16_t		m_uFlagsStorageIndex = 0;

		uint8_t			m_fFlags = 0;

		uint8_t			m_u8FlipInterval = 5;

	public:		
		explicit TurntableAutoInverterDecoder(dcclite::Packet& packet) noexcept;
		explicit TurntableAutoInverterDecoder(Storage::EpromStream& stream) noexcept;		

		bool Update(const unsigned long ticks) noexcept override;

		void SaveConfig(Storage::EpromStream& stream) noexcept override;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER;
		};

		bool IsOutputDecoder() const noexcept override
		{
			return false;
		}

		bool AcceptServerState(dcclite::DecoderStates state, const unsigned long time) noexcept override;

		bool IsActive() const noexcept override
		{
			return m_fFlags & dcclite::TRTD_ACTIVE;
		}

		bool IsSyncRequired() const noexcept override
		{
			const bool active = this->IsActive();
			const bool remoteActive = m_fFlags & dcclite::TRTD_REMOTE_ACTIVE;

			return active != remoteActive;
		}		

	private:
		void Init(const dcclite::PinType_t trackPins[4]) noexcept;

		void TurnOnTrackPower() noexcept;
};
