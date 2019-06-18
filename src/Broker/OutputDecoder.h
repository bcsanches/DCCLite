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
		OutputDecoder(const Class& decoderClass,
			const Address& address,
			const std::string& name,
			IDccDecoderServices& owner,
			const rapidjson::Value& params
		) :
			Decoder(decoderClass, address, name, owner, params)
		{
			//empty
		}

		virtual void WriteConfig(dcclite::Packet& packet) const = 0;

		dcclite::DecoderTypes GetType() const noexcept override
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
		
		bool IsOutputDecoder() const override
		{
			return true;
		}

		bool IsInputDecoder() const override
		{
			return false;
		}

		std::optional<dcclite::DecoderStates> GetPendingStateChange() const override
		{
			return m_kRequestedState != this->GetRemoteState() ? std::optional{ m_kRequestedState } : std::nullopt;
		}				

		//
		//IObject
		//
		//

		virtual const char* GetTypeName() const noexcept
		{
			return "OutputDecoder";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t& stream) const
		{
			Decoder::Serialize(stream);			
		}		

	private:				
		dcclite::DecoderStates m_kRequestedState = dcclite::DecoderStates::INACTIVE;
};