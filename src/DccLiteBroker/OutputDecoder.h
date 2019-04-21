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

class OutputDecoder : public Decoder
{
	public:
		OutputDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			DccLiteService &owner,
			const rapidjson::Value &params
		);

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept
		{
			return dcclite::DecoderTypes::DEC_OUTPUT;
		}

		void Activate()
		{
			m_kRequestedState = dcclite::DecoderStates::ACTIVE;
		}

		void Deactivate()
		{
			m_kRequestedState = dcclite::DecoderStates::INACTIVE;
		}

		void ToggleState()
		{
			m_kRequestedState = m_kRequestedState == dcclite::DecoderStates::ACTIVE ? dcclite::DecoderStates::INACTIVE : dcclite::DecoderStates::ACTIVE;
		}

		dcclite::DecoderStates GetRequestedState() const
		{
			return m_kRequestedState;
		}

		dcclite::DecoderStates GetCurrentState() const
		{
			return m_kCurrentState;
		}

		virtual bool IsOutputDecoder() const
		{
			return true;
		}

		virtual bool IsInputDecoder() const
		{
			return false;
		}

		virtual std::optional<dcclite::DecoderStates> GetPendingStateChange() const
		{
			return m_kRequestedState != m_kCurrentState ? std::optional{ m_kRequestedState } : std::nullopt;
		}

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "Device";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Decoder::Serialize(stream);

			stream.AddIntValue ("pin", m_iPin);
			stream.AddBool("invertedOperation", m_fInvertedOperation);
			stream.AddBool("ignoreSaveState", m_fIgnoreSavedState);
			stream.AddBool("activateOnPowerUp", m_fActivateOnPowerUp);
		}

	private:
		dcclite::PinType_t m_iPin;

		bool m_fInvertedOperation = false;
		bool m_fIgnoreSavedState = false;
		bool m_fActivateOnPowerUp = false;

		dcclite::DecoderStates m_kCurrentState = dcclite::DecoderStates::INACTIVE;
		dcclite::DecoderStates m_kRequestedState = dcclite::DecoderStates::INACTIVE;
};