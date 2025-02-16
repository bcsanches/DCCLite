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

		m_uActivateDelay = dcclite::json::TryGetDefaultInt(params, "activateDelay", 0) * 1000;
		m_uDeactivateDelay = dcclite::json::TryGetDefaultInt(params, "deactivateDelay", 0) * 1000;

		m_uActivateDelay = dcclite::json::TryGetDefaultInt(params, "activateDelayMs", m_uActivateDelay);
		m_uDeactivateDelay = dcclite::json::TryGetDefaultInt(params, "deactivateDelayMs", m_uDeactivateDelay);
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

		//Old devices? Send a byte only with a delay in seconds...
		if (m_rclDevice.TryGetINetworkDevice()->GetProtocolVersion() == dcclite::PROTOCOL_VERSION9)
		{
			uint8_t activateDelay = m_uActivateDelay / 1000;
			uint8_t deactivateDelay = m_uDeactivateDelay / 1000;

			//make sure if activate or deactivate has something (less than 1000), that we have at least 1 sec
			activateDelay = ((activateDelay == 0) && m_uActivateDelay) ? 1 : activateDelay;			
			deactivateDelay = ((deactivateDelay == 0) && m_uDeactivateDelay) ? 1 : deactivateDelay;
				
			packet.Write8(activateDelay);
			packet.Write8(deactivateDelay);
		}
		else
		{
			//new devices handles milliseconds (16 bits) delay
			packet.Write16(m_uActivateDelay);
			packet.Write16(m_uDeactivateDelay);
		}		
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
