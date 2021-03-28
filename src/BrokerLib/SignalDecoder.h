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

#include <set>

#include "EmbeddedLibDefs.h"
#include "NmraUtil.h"

class SignalDecoder : public Decoder
{
	public:
		SignalDecoder(			
			const DccAddress &address,
			const std::string &name,
			IDccLite_DecoderServices &owner,
			IDevice_DecoderServices &dev,
			const rapidjson::Value &params
		);

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
		struct Aspect
		{			
			dcclite::SignalAspects m_eAspect;

			std::vector<std::string> m_vecOnHeads;
			std::vector<std::string> m_vecOffHeads;

			bool m_Flash = false;
		};

		friend class SignalTester;

	private:				
		dcclite::DecoderStates m_kRequestedState = dcclite::DecoderStates::INACTIVE;

		std::map<std::string, std::string> m_mapHeads;
		std::vector<Aspect> m_vecAspects;
};
