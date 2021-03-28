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

#include "IDevice.h"
#include "Packet.h"


SimpleOutputDecoder::SimpleOutputDecoder(	
	const DccAddress &address,
	const std::string &name,
	IDccLite_DecoderServices &owner,
	IDevice_DecoderServices &dev,
	const rapidjson::Value &params
) :
	OutputDecoder(address, name, owner, dev, params),
	m_clPin(params["pin"].GetInt())
{	
	m_rclDevice.TryGetINetworkDevice()->Decoder_RegisterPin(*this, m_clPin, "pin");

	auto inverted = params.FindMember("inverted");	
	m_fInvertedOperation = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

	auto setOnPower = params.FindMember("ignoreSavedState");
	m_fIgnoreSavedState = setOnPower != params.MemberEnd() ? setOnPower->value.GetBool() : false;

	auto activateOnPowerUp = params.FindMember("activateOnPowerUp");
	m_fActivateOnPowerUp = activateOnPowerUp != params.MemberEnd() ? activateOnPowerUp->value.GetBool() : false;

	this->SyncRemoteState(m_fIgnoreSavedState && m_fActivateOnPowerUp ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);
}

SimpleOutputDecoder::~SimpleOutputDecoder()
{
	m_rclDevice.TryGetINetworkDevice()->Decoder_UnregisterPin(*this, m_clPin);
}

uint8_t SimpleOutputDecoder::GetDccppFlags() const noexcept
{
	return	(m_fInvertedOperation ? dcclite::OutputDecoderFlags::OUTD_INVERTED_OPERATION : 0) |
			(m_fIgnoreSavedState ? dcclite::OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE : 0) |
			(m_fActivateOnPowerUp ? dcclite::OUTD_ACTIVATE_ON_POWER_UP : 0);
}


void SimpleOutputDecoder::WriteConfig(dcclite::Packet &packet) const
{
	OutputDecoder::WriteConfig(packet);	

	packet.Write8(m_clPin.Raw());
	packet.Write8(this->GetDccppFlags());		
}
