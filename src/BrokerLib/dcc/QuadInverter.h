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

#include "BasicPin.h"

namespace dcclite::broker
{	
	class QuadInverter : public OutputDecoder
	{
		public:
			QuadInverter(
				const DccAddress &address,
				const std::string &name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			~QuadInverter() override;			

			void WriteConfig(dcclite::Packet &packet) const override;

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_QUAD_INVERTER;
			}		
			
			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "QuadInverterDecoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:
			dcclite::BasicPin m_arTrackAPins[2];
			dcclite::BasicPin m_arTrackBPins[2];

			uint8_t		m_u8FlipInterval = 5;

			bool m_fIgnoreSavedState = false;
			bool m_fActivateOnPowerUp = false;
	};

}
