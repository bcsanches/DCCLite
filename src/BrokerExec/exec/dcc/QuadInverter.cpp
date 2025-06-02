// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "QuadInverter.h"

#include <dcclite/JsonUtils.h>

#include <dcclite_shared/Packet.h>

#include "IDevice.h"

namespace dcclite::broker::exec::dcc
{

	QuadInverter::QuadInverter(
		const Address &address,
		RName name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		OutputDecoder(address, name, owner, dev, params)		
	{						
		const auto trackAPins = json::GetArray(params, "trackPowerAPins", "QuadInverter");
		const auto trackBPins = json::GetArray(params, "trackPowerBPins", "QuadInverter");

		m_arTrackAPins[0] = dcclite::BasicPin{ static_cast<PinType_t>(trackAPins[0].GetInt()) };
		m_arTrackAPins[1] = dcclite::BasicPin{ static_cast<PinType_t>(trackAPins[1].GetInt()) };

		m_arTrackBPins[0] = dcclite::BasicPin{ static_cast<PinType_t>(trackBPins[0].GetInt()) };
		m_arTrackBPins[1] = dcclite::BasicPin{ static_cast<PinType_t>(trackBPins[1].GetInt()) };

		auto networkDevice = m_rclDevice.TryGetINetworkDevice();		

		networkDevice->Decoder_RegisterPin(*this, m_arTrackAPins[0], "trackA0");
		networkDevice->Decoder_RegisterPin(*this, m_arTrackAPins[1], "trackA1");

		networkDevice->Decoder_RegisterPin(*this, m_arTrackBPins[0], "trackB0");
		networkDevice->Decoder_RegisterPin(*this, m_arTrackBPins[1], "trackB1");		

		m_u8FlipInterval = json::TryGetDefaultInt(params, "flipInterval", m_u8FlipInterval);
	}

	QuadInverter::~QuadInverter()
	{
		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		networkDevice->Decoder_UnregisterPin(*this, m_arTrackAPins[0]);
		networkDevice->Decoder_UnregisterPin(*this, m_arTrackAPins[1]);

		networkDevice->Decoder_UnregisterPin(*this, m_arTrackBPins[0]);
		networkDevice->Decoder_UnregisterPin(*this, m_arTrackBPins[1]);
	}

	void QuadInverter::WriteConfig(dcclite::Packet &packet) const
	{
		OutputDecoder::WriteConfig(packet);

		const uint8_t flags = 
			(this->InvertedOperation() ? dcclite::QUAD_INVERTED : 0) |
			(this->IgnoreSavedState() ? dcclite::QUAD_IGNORE_SAVED_STATE : 0) | 
			(this->ActivateOnPowerUp() ? dcclite::QUAD_ACTIVATE_ON_POWER_UP : 0);
		
		packet.Write8(flags);	
		packet.Write8(m_u8FlipInterval);

		packet.Write8(m_arTrackAPins[0].Raw());
		packet.Write8(m_arTrackAPins[1].Raw());
		packet.Write8(m_arTrackBPins[0].Raw());
		packet.Write8(m_arTrackBPins[1].Raw());
	}

	void QuadInverter::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		OutputDecoder::Serialize(stream);

		stream.AddIntValue("flipInterval", m_u8FlipInterval);

		stream.AddIntValue("trackA0Pin", m_arTrackAPins[0].Raw());
		stream.AddIntValue("trackA1Pin", m_arTrackAPins[1].Raw());

		stream.AddIntValue("trackB0Pin", m_arTrackBPins[0].Raw());
		stream.AddIntValue("trackB1Pin", m_arTrackBPins[1].Raw());
	}
}
