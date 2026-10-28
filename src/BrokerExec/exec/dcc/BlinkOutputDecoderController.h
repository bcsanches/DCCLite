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

namespace dcclite::broker::exec::dcc
{
	class BlinkOutputDecoderController : public OutputDecoder
	{
		public:
			BlinkOutputDecoderController(
				const Address &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

		~BlinkOutputDecoderController() = default;

		void InitAfterDeviceLoad() override;

		void WriteConfig(dcclite::Packet &packet) const override;

		dcclite::DecoderTypes GetType() const noexcept override
		{
			return dcclite::DecoderTypes::DEC_BLINK_OUTPUT_CONTROLLER;
		}

		//
		//IObject
		//
		//

		const char *GetTypeName() const noexcept override
		{
			return "BlinkOutputDecoderController";
		}

		private:
			RName	m_rnTarget;

			uint8_t m_u8TargetIndex = 0;
	};
}
