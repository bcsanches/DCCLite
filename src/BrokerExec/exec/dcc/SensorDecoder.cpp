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

#include <dcclite_shared/Packet.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>


#include "IDevice.h"

namespace dcclite::broker::exec::dcc
{

	SensorDecoder::SensorDecoder(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		RemoteDecoder(address, name, owner, dev, params),
		m_clPin(params["pin"].GetInt())
	{
		m_rclDevice.TryGetINetworkDevice()->Decoder_RegisterPin(*this, m_clPin, "pin");

		m_fPullUp = dcclite::json::TryGetDefaultBool(params, "pullup", false);
		m_fInverted = dcclite::json::TryGetDefaultBool(params, "inverted", false);

		if (params.HasMember("activateDelay") && params.HasMember("activateDelayMs"))
		{
			dcclite::Log::Warn("[SensorDecoder::SensorDecoder] Decoder {} contains both activateDelay and activateDelayMs", this->GetName());
		}

		if (params.HasMember("deactivateDelay") && params.HasMember("deactivateDelayMs"))
		{
			dcclite::Log::Warn("[SensorDecoder::SensorDecoder] Decoder {} contains both deactivateDelay and deactivateDelayMs", this->GetName());
		}

		if (params.HasMember("startDelay") && params.HasMember("startDelayMs"))
		{
			dcclite::Log::Warn("[SensorDecoder::SensorDecoder] Decoder {} contains both startDelay and startDelayMs", this->GetName());
		}

		m_uActivateDelay = dcclite::json::TryGetDefaultInt(params, "activateDelay", 0) * 1000;
		m_uDeactivateDelay = dcclite::json::TryGetDefaultInt(params, "deactivateDelay", 0) * 1000;
		m_uStartDelay = dcclite::json::TryGetDefaultInt(params, "startDelay", 0) * 1000;

		m_uActivateDelay = dcclite::json::TryGetDefaultInt(params, "activateDelayMs", m_uActivateDelay);
		m_uDeactivateDelay = dcclite::json::TryGetDefaultInt(params, "deactivateDelayMs", m_uDeactivateDelay);
		m_uStartDelay = dcclite::json::TryGetDefaultInt(params, "startDelayMs", m_uStartDelay);
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
		
		packet.Write16(m_uActivateDelay);
		packet.Write16(m_uDeactivateDelay);
		packet.Write16(m_uStartDelay);
	}

	void SensorDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		RemoteDecoder::Serialize(stream);

		stream.AddIntValue("pin", m_clPin.Raw());
		stream.AddBool("pullUp", m_fPullUp);
		stream.AddBool("inverted", m_fInverted);
		stream.AddIntValue("activateDelay", m_uActivateDelay);
		stream.AddIntValue("deactivateDelay", m_uDeactivateDelay);
		stream.AddIntValue("startDelay", m_uStartDelay);
	}
}
