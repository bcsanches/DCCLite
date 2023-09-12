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

#include "BasicPin.h"

namespace dcclite::broker
{
	class SensorDecoder;

	class TurntableAutoInverterDecoder : public RemoteDecoder
	{
		public:
			TurntableAutoInverterDecoder(
				const DccAddress &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			~TurntableAutoInverterDecoder() override;

			void InitAfterDeviceLoad() override;

			void WriteConfig(dcclite::Packet &packet) const override;

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER;
			}

			bool IsOutputDecoder() const override
			{
				return false;
			}

			bool IsInputDecoder() const override
			{
				return true;
			}		
			
			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "TurntableAutoInverterDecoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:
			RName m_rnSensorAName;
			RName m_rnSensorBName;

			dcclite::BasicPin m_arTrackAPins[2];
			dcclite::BasicPin m_arTrackBPins[2];

			uint8_t		m_u8SensorAIndex = 0;
			uint8_t		m_u8SensorBIndex = 0;		

			uint8_t		m_u8FlipInterval = 5;
	};

}
