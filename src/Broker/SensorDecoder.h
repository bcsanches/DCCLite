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

#include "BasicPin.h"

class SensorDecoder : public Decoder
{
	public:
		SensorDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			IDccDecoderServices &owner,
			const rapidjson::Value &params
		);

		void WriteConfig(dcclite::Packet &packet) const override;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_SENSOR;
		}

		bool IsOutputDecoder() const override
		{
			return false;
		}

		bool IsInputDecoder() const override
		{
			return true;
		}						

		inline dcclite::PinType_t GetPin() const
		{
			return m_clPin.Raw();
		}

		inline bool HasPullUp() const
		{
			return m_fPullUp;
		}

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "SensorDecoder";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Decoder::Serialize(stream);

			stream.AddIntValue("pin", m_clPin.Raw());
			stream.AddBool("pullup", m_fPullUp);	
			stream.AddBool("inverted", m_fInverted);
		}	

	private:
		dcclite::BasicPin m_clPin;

		bool m_fPullUp = false;		
		bool m_fInverted = false;
};
