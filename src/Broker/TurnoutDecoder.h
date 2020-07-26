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

#include <chrono>

#include "BasicPin.h"
#include "OutputDecoder.h"

class TurnoutDecoder : public OutputDecoder
{
	public:
		TurnoutDecoder(const Class& decoderClass,
			const DccAddress& address,
			const std::string& name,
			IDccDecoderServices& owner,
			Device &dev,
			const rapidjson::Value& params
		):
			OutputDecoder(decoderClass, address, name, owner, dev, params)
		{
			//empty
		}

		bool IsTurnoutDecoder() const override
		{
			return true;
		}

		const char *GetTypeName() const noexcept override
		{
			return "TurnoutDecoder";
		}
};

class ServoTurnoutDecoder : public TurnoutDecoder
{
	public:
		ServoTurnoutDecoder(const Class& decoderClass,
			const DccAddress& address,
			const std::string& name,
			IDccDecoderServices& owner,
			Device &dev,
			const rapidjson::Value& params
		);

		void WriteConfig(dcclite::Packet& packet) const override;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_SERVO_TURNOUT;
		}

		void Serialize(dcclite::JsonOutputStream_t& stream) const override
		{
			OutputDecoder::Serialize(stream);

			stream.AddIntValue("pin", m_clPin.Raw());

			if (m_clPowerPin)
				stream.AddIntValue("powerPin", m_clPowerPin.Raw());

			if (m_clFrogPin)
				stream.AddIntValue("frogPin", m_clFrogPin.Raw());

			stream.AddBool("invertedOperation", m_fInvertedOperation);
			stream.AddBool("ignoreSaveState", m_fIgnoreSavedState);
			stream.AddBool("activateOnPowerUp", m_fActivateOnPowerUp);
			stream.AddBool("invertedFrog", m_fInvertedFrog);
		}

		const char *GetTypeName() const noexcept override
		{
			return "ServoTurnoutDecoder";
		}


	private:
		dcclite::BasicPin	m_clPin;
		dcclite::BasicPin	m_clPowerPin;
		dcclite::BasicPin	m_clFrogPin;

		std::uint8_t				m_uRange = dcclite::SERVO_DEFAULT_RANGE;
		std::chrono::milliseconds	m_tOperationTime = std::chrono::milliseconds{1000};

		bool m_fInvertedOperation = false;
		bool m_fIgnoreSavedState = false;
		bool m_fActivateOnPowerUp = false;

		bool m_fInvertedFrog = false;		
};
