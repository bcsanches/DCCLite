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

#include "TurnoutDecoder.h"

namespace dcclite::broker::exec::dcc
{
	/*
	* Useful decode for exposing a virtual button on JMRI. 
	* 
	* Can be used to trigger Lua scripts, for example.
	* 
	*/
	class VirtualTurnoutDecoder : public TurnoutDecoder
	{
		public:
			VirtualTurnoutDecoder(
				const Address &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value& params
			):
				TurnoutDecoder{ address, name, owner, dev, params }
			{
				//empty
			}	

			const char *GetTypeName() const noexcept override
			{
				return "VirtualTurnoutDecoder";
			}
	};
}
