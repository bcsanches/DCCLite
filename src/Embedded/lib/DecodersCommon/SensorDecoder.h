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

#define CFG_COOLDOWN_TIMEOUT_TICKS 25

class SensorDecoder : public Decoder
{
	private:		
		unsigned long m_uCoolDownTicks = 0;
		
		uint16_t		m_uActivateDelay = 0;
		uint16_t		m_uDeactivateDelay = 0;		
		uint16_t		m_uStartDelay = 0;

		Pin				m_clPin;
		uint8_t			m_fFlags = 0;

	public:
		explicit SensorDecoder(uint8_t flags, dcclite::PinType_t pin, uint16_t activateDelay = 0, uint16_t deactivateDelay = 0, uint16_t startDelay = 0) noexcept;
		explicit SensorDecoder(dcclite::Packet& packet) noexcept;
		explicit SensorDecoder(Storage::EpromStream& stream) noexcept;

		bool Update(const unsigned long ticks) noexcept override;

		void SaveConfig(Storage::EpromStream& stream) noexcept override;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_SENSOR;
		};

		bool IsOutputDecoder() const noexcept override
		{
			return false;
		}

		bool AcceptServerState(dcclite::DecoderStates state, const unsigned long ticks) noexcept override;

		bool IsActive() const noexcept override
		{
			return m_fFlags & dcclite::SNRD_ACTIVE;
		}

		bool IsSyncRequired() const noexcept override
		{
			bool active = this->IsActive();
			bool remoteActive = m_fFlags & dcclite::SNRD_REMOTE_ACTIVE;

			return active != remoteActive;
		}

	private:
		void Init(const dcclite::PinType_t pin) noexcept;
};
