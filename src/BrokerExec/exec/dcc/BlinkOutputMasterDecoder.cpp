// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "BlinkOutputMasterDecoder.h"

#include <dcclite_shared/Packet.h>

#include "BlinkOutputDecoder.h"

#include <dcclite/JsonUtils.h>

namespace dcclite::broker::exec::dcc
{
	BlinkOutputMasterDecoder::BlinkOutputMasterDecoder(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	):
		OutputDecoder{ address, name, owner, dev, params },
		m_rnTarget{ dcclite::json::GetString(params, "target", "BlinkOutputMasterDecoder") }
	{
		//empty
	}

	void BlinkOutputMasterDecoder::InitAfterDeviceLoad()
	{
		RemoteDecoder::InitAfterDeviceLoad();

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		auto &decoder = networkDevice->FindDecoder(m_rnTarget);

		//make sure we are pointing to an blink output decoder
		auto &outputDecoder = dynamic_cast<BlinkOutputDecoder &>(decoder);

		m_u8TargetIndex = networkDevice->FindDecoderIndex(outputDecoder);
	}

	void BlinkOutputMasterDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		OutputDecoder::WriteConfig(packet);

		uint8_t flags = this->ActivateOnPowerUp() ? BOUTMD_ACTIVATE_ON_POWER_UP : 0;
		flags |= this->IgnoreSavedState() ? BOUTMD_IGNORE_SAVED_STATE : 0;

		packet.Write8(flags);
		packet.Write8(m_u8TargetIndex);
	}
}
