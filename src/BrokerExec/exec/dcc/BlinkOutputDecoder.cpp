// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "BlinkOutputDecoder.h"

#include <dcclite_shared/Packet.h>

#include "IDevice.h"
#include <dcclite/FmtUtils.h>

namespace dcclite::broker::exec::dcc
{
	BlinkOutputDecoder::BlinkOutputDecoder(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		SimpleOutputDecoder(address, name, owner, dev, params)
	{
		auto blinkInterval = params.FindMember("blinkInterval");
		m_tBlinkInterval = blinkInterval != params.MemberEnd() ? std::chrono::milliseconds{ blinkInterval->value.GetUint() } : m_tBlinkInterval;

		if (m_tBlinkInterval.count() > std::numeric_limits<uint16_t>::max())
		{
			throw std::logic_error(
				fmt::format(
					"[BlinkOutputDecoder::{}] blinkInterval is too big ({}), max is {}.",
					this->GetName(),
					m_tBlinkInterval.count(), 
					std::numeric_limits<uint16_t>::max()
				)
			);
		}
	}	

	void BlinkOutputDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		SimpleOutputDecoder::WriteConfig(packet);

		packet.Write16(static_cast<uint16_t>(m_tBlinkInterval.count()));
	}
}