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

#include <dcclite_shared/SharedLibDefs.h>

namespace dcclite::broker::exec::dcc
{
	class StateDecoder : public Decoder
	{
		public:
			StateDecoder(
				const Address &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			inline dcclite::DecoderStates GetState() const
			{
				return m_kState;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			virtual bool IsOutputDecoder() const = 0;
			virtual bool IsInputDecoder() const = 0;

			virtual bool IsTurnoutDecoder() const
			{
				return false;
			}

		protected:
			bool SetState(dcclite::DecoderStates state, bool publishUpdate = true);

		private:
			dcclite::DecoderStates m_kState = dcclite::DecoderStates::INACTIVE;
	};	
}


