// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SimpleOutputDecoder.h"

#include "Packet.h"

static Decoder::Class simpleOutputDecoder("Output",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, const std::string &name, IDccDecoderServices &owner, const rapidjson::Value &params)
		-> std::unique_ptr<Decoder> { return std::make_unique<SimpleOutputDecoder>(decoderClass, address, name, owner, params); }
);


SimpleOutputDecoder::SimpleOutputDecoder(
	const Class &decoderClass,
	const Address &address,
	const std::string &name,
	IDccDecoderServices &owner,
	const rapidjson::Value &params
) :
	OutputDecoder(decoderClass, address, name, owner, params),
	m_clPin(params["pin"].GetInt())
{	
	auto inverted = params.FindMember("inverted");	
	m_fInvertedOperation = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

	auto setOnPower = params.FindMember("ignoreSavedState");
	m_fIgnoreSavedState = setOnPower != params.MemberEnd() ? setOnPower->value.GetBool() : false;

	auto activateOnPowerUp = params.FindMember("activateOnPowerUp");
	m_fActivateOnPowerUp = activateOnPowerUp != params.MemberEnd() ? activateOnPowerUp->value.GetBool() : false;

	this->SyncRemoteState(m_fIgnoreSavedState && m_fActivateOnPowerUp ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);
}


void SimpleOutputDecoder::WriteConfig(dcclite::Packet &packet) const
{
	Decoder::WriteConfig(packet);	

	packet.Write8(m_clPin.Raw());
	packet.Write8(
		(m_fInvertedOperation ? dcclite::OutputDecoderFlags::OUTD_INVERTED_OPERATION : 0) |
		(m_fIgnoreSavedState ? dcclite::OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE : 0) |
		(m_fActivateOnPowerUp ? dcclite::OUTD_ACTIVATE_ON_POWER_UP : 0)
	);
}