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

#include "EmbeddedLibDefs.h"
#include "BasicPin.h"

class SimpleOutputDecoder : public OutputDecoder
{
	public:
		SimpleOutputDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			IDccDecoderServices &owner,
			const rapidjson::Value &params
		);

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept
		{
			return dcclite::DecoderTypes::DEC_OUTPUT;
		}
		
		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "SimpleOutputDecoder";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			OutputDecoder::Serialize(stream);

			stream.AddIntValue ("pin", m_clPin.Raw());
			stream.AddBool("invertedOperation", m_fInvertedOperation);
			stream.AddBool("ignoreSaveState", m_fIgnoreSavedState);
			stream.AddBool("activateOnPowerUp", m_fActivateOnPowerUp);
		}

	private:
		dcclite::BasicPin m_clPin;

		bool m_fInvertedOperation = false;
		bool m_fIgnoreSavedState = false;
		bool m_fActivateOnPowerUp = false;		
};