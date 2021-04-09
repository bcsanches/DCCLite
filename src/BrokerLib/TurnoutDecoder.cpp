// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TurnoutDecoder.h"

#include "IDevice.h"

#include <Packet.h>

ServoTurnoutDecoder::ServoTurnoutDecoder(
	const DccAddress& address,
	const std::string& name,
	IDccLite_DecoderServices &owner,
	IDevice_DecoderServices &dev,
	const rapidjson::Value& params
) :
	TurnoutDecoder(address, name, owner, dev, params),
	m_clPin(params["pin"].GetInt())
{
	auto networkDevice = m_rclDevice.TryGetINetworkDevice();

	assert(networkDevice);

	networkDevice->Decoder_RegisterPin(*this, m_clPin, "pin");

	auto powerPin = params.FindMember("powerPin");
	if (powerPin != params.MemberEnd())
	{
		m_clPowerPin = dcclite::BasicPin{ static_cast<dcclite::PinType_t>(powerPin->value.GetInt())};
		networkDevice->Decoder_RegisterPin(*this, m_clPowerPin, "powerPin");
	}	

	auto frogPin = params.FindMember("frogPin");
	if(frogPin != params.MemberEnd())
	{
		m_clFrogPin = dcclite::BasicPin{ static_cast<dcclite::PinType_t>(frogPin->value.GetInt()) };
		networkDevice->Decoder_RegisterPin(*this, m_clFrogPin, "frogPin");
	}

	auto inverted = params.FindMember("inverted");
	m_fInvertedOperation = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

	auto setOnPower = params.FindMember("ignoreSavedState");
	m_fIgnoreSavedState = setOnPower != params.MemberEnd() ? setOnPower->value.GetBool() : false;

	auto activateOnPowerUp = params.FindMember("activateOnPowerUp");
	m_fActivateOnPowerUp = activateOnPowerUp != params.MemberEnd() ? activateOnPowerUp->value.GetBool() : false;

	auto invertedFrog = params.FindMember("invertedFrog");
	m_fInvertedFrog = invertedFrog != params.MemberEnd() ? invertedFrog->value.GetBool() : false;

	auto invertedPower = params.FindMember("invertedPower");
	m_fInvertedPower = invertedPower != params.MemberEnd() ? invertedPower->value.GetBool() : false;

	auto range = params.FindMember("range");
	m_uRange = range != params.MemberEnd() ? range->value.GetUint() : m_uRange;

	auto operationTime = params.FindMember("operationTime");
	m_tOperationTime = operationTime != params.MemberEnd() ? std::chrono::milliseconds{ operationTime->value.GetUint() } : m_tOperationTime;

	this->SyncRemoteState(m_fIgnoreSavedState && m_fActivateOnPowerUp ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);
}

ServoTurnoutDecoder::~ServoTurnoutDecoder()
{
	auto networkDevice = m_rclDevice.TryGetINetworkDevice();

	assert(networkDevice);

	networkDevice->Decoder_UnregisterPin(*this, m_clPin);
	networkDevice->Decoder_UnregisterPin(*this, m_clPowerPin);
	networkDevice->Decoder_UnregisterPin(*this, m_clFrogPin);
}

void ServoTurnoutDecoder::WriteConfig(dcclite::Packet& packet) const
{
	TurnoutDecoder::WriteConfig(packet);

	packet.Write8(m_clPin.Raw());	

	packet.Write8(
		(m_fInvertedOperation ? dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_OPERATION : 0) |
		(m_fIgnoreSavedState ? dcclite::ServoTurnoutDecoderFlags::SRVT_IGNORE_SAVED_STATE : 0) |
		(m_fActivateOnPowerUp ? dcclite::ServoTurnoutDecoderFlags::SRVT_ACTIVATE_ON_POWER_UP : 0) |
		(m_fInvertedFrog ? dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_FROG : 0) |
		(m_fInvertedPower ? dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_POWER : 0)
	);

	packet.Write8(m_clPowerPin.Raw());
	packet.Write8(m_clFrogPin.Raw());
	packet.Write8(m_uRange);

	auto ticks = m_tOperationTime.count() / m_uRange;
	packet.Write8(ticks > 255 ? 255 : static_cast<uint8_t>(ticks));
}

