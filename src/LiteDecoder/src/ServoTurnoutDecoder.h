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

		enum class States: uint8_t
		{
			CLOSED,
			CLOSING,
			THROWN,
			THROWNING
		};

		States	m_kState;
		uint8_t m_uServoPos;

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

		bool AcceptServerState(const dcclite::DecoderStates decoderState);

		virtual bool IsActive() const
		{
			return this->State2DecoderState() == dcclite::DecoderStates::ACTIVE;
		}

		virtual bool IsSyncRequired() const
		{
			return false;
		}

		bool Update(const unsigned long ticks) override;

	private:
		void Init(const dcclite::PinType_t powerPin, const dcclite::PinType_t frogPin);
		
		void TurnOnPower(const unsigned long ticks);
		void TurnOffPower();

		States GetState() const;
		void SetState(const States newState);

		inline States GetStateGoal() const
		{
			const auto state = this->GetState();

			if (state == States::CLOSING)
				return States::CLOSED;
			
			if (state == States::THROWNING)
				return States::THROWN;

			return state;
		}				

		void OperateThrown(const unsigned long ticks);
		void OperateClose(const unsigned long ticks);

		States DecoderState2State(dcclite::DecoderStates state) const;
		dcclite::DecoderStates State2DecoderState() const;

		bool StateUpdate(const uint8_t desiredPosition, const States desiredState, const int moveDirection, const unsigned long ticks);
};
