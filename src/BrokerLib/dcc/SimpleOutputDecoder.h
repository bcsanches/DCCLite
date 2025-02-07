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

#include "SharedLibDefs.h"
#include "BasicPin.h"

namespace dcclite::broker
{

	class SimpleOutputDecoder : public OutputDecoder
	{
		public:
			SimpleOutputDecoder(
				const DccAddress &address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			~SimpleOutputDecoder() override;

			void WriteConfig(dcclite::Packet &packet) const override;

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_OUTPUT;
			}

			inline dcclite::BasicPin GetPin() const noexcept
			{
				return m_clPin;
			}

			uint8_t GetDccppFlags() const noexcept;
		
			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "SimpleOutputDecoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override
			{
				OutputDecoder::Serialize(stream);

				stream.AddIntValue ("pin", m_clPin.Raw());				
			}

		private:
			dcclite::BasicPin m_clPin;			
	};

}