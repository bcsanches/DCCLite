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

		unsigned long m_uCoolDownTicks = 0;

	public:
		SensorDecoder(dcclite::Packet& packet);
		SensorDecoder(EpromStream& stream);

		virtual bool Update(const unsigned long ticks);

		virtual void SaveConfig(EpromStream& stream);

		virtual dcclite::DecoderTypes GetType() const
		{
			return dcclite::DecoderTypes::DEC_SENSOR;
		};

		bool IsOutputDecoder() const override
		{
			return false;
		}

		bool AcceptServerState(dcclite::DecoderStates state);

		virtual bool IsActive() const
		{
			return m_fFlags & dcclite::SNRD_ACTIVE;
		}

		virtual bool IsSyncRequired() const
		{
			bool active = this->IsActive();
			bool remoteActive = m_fFlags & dcclite::SNRD_REMOTE_ACTIVE;

			return active != remoteActive;
		}

	private:
		void Init(const dcclite::PinType_t pin);
};
