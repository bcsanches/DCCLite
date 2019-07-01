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

#include <Arduino.h> 
#include <Servo.h>

#include "Pin.h"

class ServoTurnoutDecoder : public Decoder
{
	private:
		Servo			m_clServo;

		unsigned long 	m_uNextThink = 0;


		uint16_t			m_uFlagsStorageIndex = 0;		
		dcclite::BasicPin	m_clPin;
		uint8_t				m_fFlags = 0;		

		Pin			m_clPowerPin;
		Pin			m_clFrogPin;		

		uint8_t		m_uRange;
		uint8_t		m_uTicks;

	public:
		ServoTurnoutDecoder(dcclite::Packet& packet);
		ServoTurnoutDecoder(EpromStream& stream);
		~ServoTurnoutDecoder();

		virtual void SaveConfig(EpromStream& stream);

		virtual dcclite::DecoderTypes GetType() const
		{
			return dcclite::DecoderTypes::DEC_SERVO_TURNOUT;
		};

		bool IsOutputDecoder() const override
		{
			return true;
		}

		bool AcceptServerState(dcclite::DecoderStates state);

		virtual bool IsActive() const
		{
			return m_fFlags & dcclite::SRVT_ACTIVE;
		}

		virtual bool IsSyncRequired() const
		{
			return false;
		}

		bool Update(const unsigned long ticks) override;

	private:
		void Init(const dcclite::PinType_t powerPin, const dcclite::PinType_t frogPin);

		void OperatePin();

		void TurnOnPower();
		void TurnOffPower();
};
