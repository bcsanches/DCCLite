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
		
		uint8_t m_uServoPos;

	public:
		explicit ServoTurnoutDecoder(dcclite::Packet& packet);
		explicit ServoTurnoutDecoder(Storage::EpromStream& stream);
		explicit ServoTurnoutDecoder(uint8_t flags, dcclite::PinType_t pin, uint8_t range, uint8_t ticks, dcclite::PinType_t powerPin = dcclite::NullPin, dcclite::PinType_t frogPin = dcclite::NullPin);
		~ServoTurnoutDecoder();

		void SaveConfig(Storage::EpromStream& stream) override;

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_SERVO_TURNOUT;
		};

		bool IsOutputDecoder() const override
		{
			return true;
		}

		bool AcceptServerState(const dcclite::DecoderStates decoderState);

		bool IsActive() const override
		{
			return this->State2DecoderState() == dcclite::DecoderStates::ACTIVE;
		}

		dcclite::DecoderStates GetDecoderState() const
		{
			return this->State2DecoderState();
		}

		bool IsSyncRequired() const override
		{
			return false;
		}

		bool Update(const unsigned long ticks) override;

		inline bool IsMoving() const noexcept
		{
			const auto state = this->GetState();

			return (state == States::CLOSING) || (state == States::THROWNING);
		}

	private:
		void Init(const dcclite::PinType_t powerPin, const dcclite::PinType_t frogPin) noexcept;
		
		void TurnOnPower(const unsigned long ticks) noexcept;
		void TurnOffPower() noexcept;

		States GetState() const noexcept;
		void SetState(const States newState) noexcept;

		inline States GetStateGoal() const noexcept
		{
			const auto state = this->GetState();

			if (state == States::CLOSING)
				return States::CLOSED;
			
			if (state == States::THROWNING)
				return States::THROWN;

			return state;
		}						

		void OperateThrown(const unsigned long ticks) noexcept;
		void OperateClose(const unsigned long ticks) noexcept;

		States DecoderState2State(dcclite::DecoderStates state) const noexcept;
		dcclite::DecoderStates State2DecoderState() const noexcept;

		bool StateUpdate(const uint8_t desiredPosition, const States desiredState, const int moveDirection, const unsigned long ticks) noexcept;
};
