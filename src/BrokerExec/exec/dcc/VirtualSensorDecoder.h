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

namespace dcclite::broker::exec::dcc
{
	constexpr auto VIRTUAL_SENSOR_DECODER_CLASSNAME = "VirtualSensorDecoder";

	/**
	* 
	* Virtual Sensor Decoder
	* 
	* A decoder that only exists on the Broker side (not implemented on external devices or arduinos)
	* 
	* It is simple used to create a software managed decoder and expose it to JMRI and other systems.
	* 
	* So far used by the Dispatcher to represent a sensor for a track Section or TSection, it is automatically 
	* instatiated by the Dispatcher when track Sections are created and managed by it
	* 
	*/
	class VirtualSensorDecoder : public StateDecoder
	{
		public:
			VirtualSensorDecoder(
				const Address &address,
				RName name,
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
