// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SensorDecoder.h"

#include <Packet.h>

#include "IDevice.h"

namespace dcclite::broker
{

	SensorDecoder::SensorDecoder(
		const DccAddress &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		RemoteDecoder(address, name, owner, dev, params),
		m_clPin(params["pin"].GetInt())
	{
		m_rclDevice.TryGetINetworkDevice()->Decoder_RegisterPin(*this, m_clPin, "pin");

		auto pullup = params.FindMember("pullup");
		m_fPullUp = pullup != params.MemberEnd() ? pullup->value.GetBool() : false;

		auto inverted = params.FindMember("inverted");
		m_fInverted = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

		auto activateDelay = params.FindMember("activateDelay");
		m_uActivateDelay = activateDelay != params.MemberEnd() ? activateDelay->value.GetInt() : 0;

		auto deactivateDelay = params.FindMember("deactivateDelay");
		m_uDeactivateDelay = deactivateDelay != params.MemberEnd() ? deactivateDelay->value.GetInt() : 0;
	}

	SensorDecoder::~SensorDecoder()
	{
		m_rclDevice.TryGetINetworkDevice()->Decoder_UnregisterPin(*this, m_clPin);
	}

	void SensorDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		RemoteDecoder::WriteConfig(packet);

		packet.Write8(m_clPin.Raw());
		packet.Write8(
			(m_fPullUp ? dcclite::SensorDecoderFlags::SNRD_PULL_UP : 0) |
			(m_fInverted ? dcclite::SensorDecoderFlags::SNRD_INVERTED : 0)
		);
		packet.Write8(m_uActivateDelay);
		packet.Write8(m_uDeactivateDelay);
	}

	void SensorDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		RemoteDecoder::Serialize(stream);

		stream.AddIntValue("pin", m_clPin.Raw());
		stream.AddBool("pullUp", m_fPullUp);
		stream.AddBool("inverted", m_fInverted);
		stream.AddIntValue("activateDelay", m_uActivateDelay);
		stream.AddIntValue("deactivateDelay", m_uDeactivateDelay);
	}
}
