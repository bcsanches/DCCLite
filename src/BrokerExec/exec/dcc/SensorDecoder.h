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

#include "RemoteDecoder.h"

#include <dcclite_shared/BasicPin.h>

namespace dcclite::broker::exec::dcc
{

	class SensorDecoder : public RemoteDecoder
	{
		public:
			SensorDecoder(
				const Address &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			~SensorDecoder() override;

			void WriteConfig(dcclite::Packet &packet) const override;

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_SENSOR;
			}

			bool IsOutputDecoder() const override
			{
				return false;
			}

			bool IsInputDecoder() const override
			{
				return true;
			}						

			inline dcclite::PinType_t GetPin() const
			{
				return m_clPin.Raw();
			}

			inline bool HasPullUp() const
			{
				return m_fPullUp;
			}

			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "SensorDecoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:			
			uint16_t m_uActivateDelay;
			uint16_t m_uDeactivateDelay;
			uint16_t m_uStartDelay;

			dcclite::BasicPin m_clPin;

			bool m_fPullUp = false;		
			bool m_fInverted = false;		
	};

}
