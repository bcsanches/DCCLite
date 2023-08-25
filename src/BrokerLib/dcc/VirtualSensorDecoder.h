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

#include "StateDecoder.h"

namespace dcclite::broker
{
	constexpr auto VIRTUAL_SENSOR_DECODER_CLASSNAME = "VirtualSensorDecoder";

	class VirtualSensorDecoder : public StateDecoder
	{
		public:
			VirtualSensorDecoder(
				const DccAddress &address,
				const std::string &name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			~VirtualSensorDecoder() override;
						
			bool IsOutputDecoder() const override
			{
				return false;
			}

			bool IsInputDecoder() const override
			{
				return true;
			}						

			inline void SetSensorState(dcclite::DecoderStates state)
			{
				this->SetState(state);
			}
			
			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return VIRTUAL_SENSOR_DECODER_CLASSNAME;
			}			
	};

}
