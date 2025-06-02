// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TurntableAutoInverterDecoder.h"

#include <dcclite_shared/Packet.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>

#include "IDevice.h"
#include "SensorDecoder.h"

namespace dcclite::broker::exec::dcc
{

	TurntableAutoInverterDecoder::TurntableAutoInverterDecoder(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		RemoteDecoder(address, name, owner, dev, params),
		m_rnSensorAName{ params["sensorA"].GetString() },
		m_rnSensorBName{ params["sensorB"].GetString() }		
	{				
		if (m_rnSensorAName == m_rnSensorBName)
		{
			throw std::invalid_argument(fmt::format("[TurntableAutoInverterDecoder::{}] Sensors cannot be the same: {}", this->GetName(), m_rnSensorAName));
		}

		m_fInverted = dcclite::json::TryGetDefaultBool(params, "inverted", false);

		const auto &trackAPin = params["trackPowerAPin"];
		const auto &trackBPin = params["trackPowerBPin"];

		m_pinTrackA = dcclite::BasicPin{ static_cast<PinType_t>(trackAPin.GetInt()) };		
		m_pinTrackB = dcclite::BasicPin{ static_cast<PinType_t>(trackBPin.GetInt()) };
		
		auto flipIntervalData = params.FindMember("flipInterval");
		m_u8FlipInterval = flipIntervalData != params.MemberEnd() ? flipIntervalData->value.GetInt() : m_u8FlipInterval;

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();		

		networkDevice->Decoder_RegisterPin(*this, m_pinTrackA, "trackA");
		networkDevice->Decoder_RegisterPin(*this, m_pinTrackB, "trackB");		
	}

	TurntableAutoInverterDecoder::~TurntableAutoInverterDecoder()
	{
		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		networkDevice->Decoder_UnregisterPin(*this, m_pinTrackA);
		networkDevice->Decoder_UnregisterPin(*this, m_pinTrackB);
	}

	static uint8_t FindSensorDecoderIndex(RName name, INetworkDevice_DecoderServices &service)
	{
		auto &decoder = service.FindDecoder(name);

		//make sure it is a sensor decoder
		auto &sensorDecoder = dynamic_cast<SensorDecoder &>(decoder);

		return service.FindDecoderIndex(sensorDecoder);
	}

	void TurntableAutoInverterDecoder::InitAfterDeviceLoad()
	{
		RemoteDecoder::InitAfterDeviceLoad();

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		m_u8SensorAIndex = FindSensorDecoderIndex(m_rnSensorAName, *networkDevice);
		m_u8SensorBIndex = FindSensorDecoderIndex(m_rnSensorBName, *networkDevice);
	}

	void TurntableAutoInverterDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		RemoteDecoder::WriteConfig(packet);
		
		packet.Write8(m_fInverted ? TRTD_INVERTED : 0);
		packet.Write8(m_u8FlipInterval);

		packet.Write8(m_u8SensorAIndex);
		packet.Write8(m_u8SensorBIndex);

		packet.Write8(m_pinTrackA.Raw());
		packet.Write8(m_pinTrackB.Raw());		
	}

	void TurntableAutoInverterDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		RemoteDecoder::Serialize(stream);

		stream.AddBool("inverted", m_fInverted);

		stream.AddIntValue("flipInterval", m_u8FlipInterval);

		stream.AddStringValue("sensorAName", m_rnSensorAName.GetData());
		stream.AddStringValue("sensorBName", m_rnSensorBName.GetData());

		stream.AddIntValue("trackAPin", m_pinTrackA.Raw());
		stream.AddIntValue("trackBPin", m_pinTrackB.Raw());		
	}
}
