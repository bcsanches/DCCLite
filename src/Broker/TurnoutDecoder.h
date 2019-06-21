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

#include "OutputDecoder.h"

class TurnoutDecoder : public OutputDecoder
{
	public:
		TurnoutDecoder(const Class& decoderClass,
			const Address& address,
			const std::string& name,
			IDccDecoderServices& owner,
			const rapidjson::Value& params
		):
			OutputDecoder(decoderClass, address, name, owner, params)
		{
			//empty
		}

		bool IsTurnoutDecoder() const override
		{
			return true;
		}
};

class ServoTurnoutDecoder : public TurnoutDecoder
{
	public:
		ServoTurnoutDecoder(const Class& decoderClass,
			const Address& address,
			const std::string& name,
			IDccDecoderServices& owner,
			const rapidjson::Value& params
		);

		virtual void WriteConfig(dcclite::Packet& packet) const;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_SERVO_TURNOUT;
		}

		void Serialize(dcclite::JsonOutputStream_t& stream) const override
		{
			OutputDecoder::Serialize(stream);

			stream.AddIntValue("pin", m_iPin);

			if (m_iPowerPin != dcclite::NULL_PIN)
				stream.AddIntValue("powerPin", m_iPowerPin);

			if (m_iFrogPin != dcclite::NULL_PIN)
				stream.AddIntValue("frogPin", m_iFrogPin);

			stream.AddBool("invertedOperation", m_fInvertedOperation);
			stream.AddBool("ignoreSaveState", m_fIgnoreSavedState);
			stream.AddBool("activateOnPowerUp", m_fActivateOnPowerUp);
			stream.AddBool("invertedFrog", m_fInvertedFrog);
		}

	private:
		dcclite::PinType_t	m_iPin = dcclite::NULL_PIN;
		dcclite::PinType_t	m_iPowerPin = dcclite::NULL_PIN;
		dcclite::PinType_t	m_iFrogPin = dcclite::NULL_PIN;

		bool m_fInvertedOperation = false;
		bool m_fIgnoreSavedState = false;
		bool m_fActivateOnPowerUp = false;

		bool m_fInvertedFrog = false;		
};
