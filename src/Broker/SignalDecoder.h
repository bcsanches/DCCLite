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

class SignalDecoder : public Decoder
{
	public:
		SignalDecoder(
			const Class& decoderClass,
			const DccAddress& address,
			const std::string& name,
			IDccLite_DecoderServices & owner,
			IDevice_DecoderServices &dev,
			const rapidjson::Value& params
		) :
			Decoder(decoderClass, address, name, owner, dev, params)
		{
			//empty
		}						

		//
		//IObject
		//
		//

		const char* GetTypeName() const noexcept override
		{
			return "SignalDecoder";
		}

		void Serialize(dcclite::JsonOutputStream_t &stream) const override;

	private:				
		dcclite::DecoderStates m_kRequestedState = dcclite::DecoderStates::INACTIVE;
};
