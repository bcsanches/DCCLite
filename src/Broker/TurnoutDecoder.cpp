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

#include <Packet.h>

static Decoder::Class servoTurnoutDecoder("ServoTurnout",
	[](const Decoder::Class& decoderClass, const Decoder::Address& address, const std::string& name, IDccDecoderServices& owner, const rapidjson::Value& params)
	-> std::unique_ptr<Decoder> { return std::make_unique<ServoTurnoutDecoder>(decoderClass, address, name, owner, params); }
);

ServoTurnoutDecoder::ServoTurnoutDecoder(const Class& decoderClass,
	const Address& address,
	const std::string& name,
	IDccDecoderServices& owner,
	const rapidjson::Value& params
) :
	TurnoutDecoder(decoderClass, address, name, owner, params),
	m_clPin(params["pin"].GetInt())
{
	auto powerPin = params.FindMember("powerPin");
	m_clPowerPin = powerPin != params.MemberEnd() ? dcclite::BasicPin{ static_cast<dcclite::PinType_t>(powerPin->value.GetInt()) } : dcclite::BasicPin{};

	auto frogPin = params.FindMember("frogPin");
	m_clFrogPin = frogPin != params.MemberEnd() ? dcclite::BasicPin{ static_cast<dcclite::PinType_t>(frogPin->value.GetInt()) } : dcclite::BasicPin{};

	auto inverted = params.FindMember("inverted");
	m_fInvertedOperation = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

	auto setOnPower = params.FindMember("ignoreSavedState");
	m_fIgnoreSavedState = setOnPower != params.MemberEnd() ? setOnPower->value.GetBool() : false;

	auto activateOnPowerUp = params.FindMember("activateOnPowerUp");
	m_fActivateOnPowerUp = activateOnPowerUp != params.MemberEnd() ? activateOnPowerUp->value.GetBool() : false;

	auto invertedFrog = params.FindMember("invertedFrog");
	m_fInvertedFrog = invertedFrog != params.MemberEnd() ? invertedFrog->value.GetBool() : false;

	auto range = params.FindMember("range");
	m_uRange = range != params.MemberEnd() ? range->value.GetUint() : m_uRange;

	auto operationTime = params.FindMember("operationTime");
	m_tOperationTime = operationTime != params.MemberEnd() ? std::chrono::milliseconds{ operationTime->value.GetUint() } : m_tOperationTime;

	this->SyncRemoteState(m_fIgnoreSavedState && m_fActivateOnPowerUp ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);
}

void ServoTurnoutDecoder::WriteConfig(dcclite::Packet& packet) const
{
	Decoder::WriteConfig(packet);

	packet.Write8(m_clPin.Raw());	

	packet.Write8(
		(m_fInvertedOperation ? dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_OPERATION : 0) |
		(m_fIgnoreSavedState ? dcclite::ServoTurnoutDecoderFlags::SRVT_IGNORE_SAVED_STATE : 0) |
		(m_fActivateOnPowerUp ? dcclite::ServoTurnoutDecoderFlags::SRVT_ACTIVATE_ON_POWER_UP : 0) |
		(m_fInvertedFrog ? dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_OPERATION : 0)
	);

	packet.Write8(m_clPowerPin.Raw());
	packet.Write8(m_clFrogPin.Raw());
	packet.Write8(m_uRange);

	auto ticks = m_tOperationTime.count() / m_uRange;
	packet.Write8(ticks > 255 ? 255 : static_cast<uint8_t>(ticks));
}