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

#include "EmbeddedLibDefs.h"

class SensorDecoder : public Decoder
{
	public:
		SensorDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			DccLiteService &owner,
			const rapidjson::Value &params
		);

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept
		{
			return dcclite::DecoderTypes::DEC_SENSOR;
		}

		virtual bool IsOutputDecoder() const
		{
			return false;
		}

		virtual bool IsInputDecoder() const
		{
			return true;
		}

		virtual void SyncRemoteState(dcclite::DecoderStates state)
		{
			m_kCurrentState = state;
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

			stream.AddIntValue("pin", m_iPin);
			stream.AddBool("pullup", m_fPullUp);			
		}

	private:
		dcclite::PinType_t m_iPin = dcclite::NULL_PIN;

		bool m_fPullUp = false;

		dcclite::DecoderStates m_kCurrentState = dcclite::DecoderStates::INACTIVE;
};
