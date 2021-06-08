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

class SensorDecoder : public Decoder
{
	private:		
		Pin				m_clPin;
		uint8_t			m_fFlags = 0;
		uint8_t			m_uActivateDelay = 0;
		uint8_t			m_uDeactivateDelay = 0;

		unsigned long m_uCoolDownTicks = 0;

	public:
		explicit SensorDecoder(dcclite::Packet& packet);
		explicit SensorDecoder(Storage::EpromStream& stream);

		bool Update(const unsigned long ticks) override;

		void SaveConfig(Storage::EpromStream& stream) override;

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_SENSOR;
		};

		bool IsOutputDecoder() const override
		{
			return false;
		}

		bool AcceptServerState(dcclite::DecoderStates state) override;

		bool IsActive() const override
		{
			return m_fFlags & dcclite::SNRD_ACTIVE;
		}

		bool IsSyncRequired() const override
		{
			bool active = this->IsActive();
			bool remoteActive = m_fFlags & dcclite::SNRD_REMOTE_ACTIVE;

			return active != remoteActive;
		}

	private:
		void Init(const dcclite::PinType_t pin);		
};
