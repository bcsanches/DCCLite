// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "BlinkOutputDecoderController.h"

#include <dcclite_shared/Packet.h>

#include "BlinkOutputDecoder.h"

namespace dcclite::broker::exec::dcc
{
	BlinkOutputDecoderController::BlinkOutputDecoderController(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	):
		OutputDecoder{ address, name, owner, dev, params },
		m_rnTarget{ params["target"].GetString() }
	{
		//empty
	}

	void BlinkOutputDecoderController::InitAfterDeviceLoad()
	{
		RemoteDecoder::InitAfterDeviceLoad();

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		auto &decoder = networkDevice->FindDecoder(m_rnTarget);

		//make sure we are pointing to an blink output decoder
		auto &outputDecoder = dynamic_cast<BlinkOutputDecoder &>(decoder);

		m_u8TargetIndex = networkDevice->FindDecoderIndex(outputDecoder);
	}

	void BlinkOutputDecoderController::WriteConfig(dcclite::Packet &packet) const
	{
		OutputDecoder::WriteConfig(packet);

		packet.Write8(m_u8TargetIndex);
	}
}
