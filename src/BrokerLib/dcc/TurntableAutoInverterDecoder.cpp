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

#include <Packet.h>

#include "IDevice.h"
#include "SensorDecoder.h"

namespace dcclite::broker
{

	TurntableAutoInverterDecoder::TurntableAutoInverterDecoder(
		const DccAddress &address,
		const std::string &name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		RemoteDecoder(address, name, owner, dev, params),
		m_strSensorAName{ params["sensorA"].GetString() },
		m_strSensorBName{ params["sensorB"].GetString() }		
	{				
		if (m_strSensorAName.compare(m_strSensorBName) == 0)
		{
			throw std::invalid_argument(fmt::format("[TurntableAutoInverterDecoder::{}] Sensors cannot be the same: {}", this->GetName(), m_strSensorAName));
		}

		const auto &trackAPins = params["trackPowerAPins"].GetArray();
		const auto &trackBPins = params["trackPowerBPins"].GetArray();

		m_arTrackAPins[0] = dcclite::BasicPin{ static_cast<PinType_t>(trackAPins[0].GetInt()) };
		m_arTrackAPins[1] = dcclite::BasicPin{ static_cast<PinType_t>(trackAPins[1].GetInt()) };

		m_arTrackBPins[0] = dcclite::BasicPin{ static_cast<PinType_t>(trackBPins[0].GetInt()) };
		m_arTrackBPins[1] = dcclite::BasicPin{ static_cast<PinType_t>(trackBPins[1].GetInt()) };

		auto flipIntervalData = params.FindMember("flipInterval");
		m_u8FlipInterval = flipIntervalData != params.MemberEnd() ? flipIntervalData->value.GetInt() : m_u8FlipInterval;

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();		

		networkDevice->Decoder_RegisterPin(*this, m_arTrackAPins[0], "trackA0");
		networkDevice->Decoder_RegisterPin(*this, m_arTrackAPins[1], "trackA1");

		networkDevice->Decoder_RegisterPin(*this, m_arTrackBPins[0], "trackB0");
		networkDevice->Decoder_RegisterPin(*this, m_arTrackBPins[1], "trackB1");
	}

	TurntableAutoInverterDecoder::~TurntableAutoInverterDecoder()
	{
		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		networkDevice->Decoder_UnregisterPin(*this, m_arTrackAPins[0]);
		networkDevice->Decoder_UnregisterPin(*this, m_arTrackAPins[1]);

		networkDevice->Decoder_UnregisterPin(*this, m_arTrackBPins[0]);
		networkDevice->Decoder_UnregisterPin(*this, m_arTrackBPins[1]);
	}

	static uint8_t FindSensorDecoderIndex(const std::string_view name, INetworkDevice_DecoderServices &service)
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

		m_u8SensorAIndex = FindSensorDecoderIndex(m_strSensorAName, *networkDevice);
		m_u8SensorBIndex = FindSensorDecoderIndex(m_strSensorBName, *networkDevice);
	}

	void TurntableAutoInverterDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		RemoteDecoder::WriteConfig(packet);

		//unused flags (send it, so in future less changes and no protocol change)
		packet.Write8(0);
		packet.Write8(m_u8FlipInterval);

		packet.Write8(m_u8SensorAIndex);
		packet.Write8(m_u8SensorBIndex);

		packet.Write8(m_arTrackAPins[0].Raw());
		packet.Write8(m_arTrackAPins[1].Raw());
		packet.Write8(m_arTrackBPins[0].Raw());
		packet.Write8(m_arTrackBPins[1].Raw());
	}

	void TurntableAutoInverterDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		RemoteDecoder::Serialize(stream);

		stream.AddIntValue("flipInterval", m_u8FlipInterval);

		stream.AddStringValue("sensorAName", m_strSensorAName);
		stream.AddStringValue("sensorBName", m_strSensorBName);

		stream.AddIntValue("trackA0Pin", m_arTrackAPins[0].Raw());
		stream.AddIntValue("trackA1Pin", m_arTrackAPins[1].Raw());

		stream.AddIntValue("trackB0Pin", m_arTrackBPins[0].Raw());
		stream.AddIntValue("trackB1Pin", m_arTrackBPins[1].Raw());
	}
}
