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

static Decoder::Class sensorDecoder("Sensor",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, const std::string &name, DccLiteService &owner, const rapidjson::Value &params)
	-> std::unique_ptr<Decoder> { return std::make_unique<SensorDecoder>(decoderClass, address, name, owner, params); }
);

SensorDecoder::SensorDecoder(const Class &decoderClass,
	const Address &address,
	const std::string &name,
	DccLiteService &owner,
	const rapidjson::Value &params
):
	Decoder(decoderClass, address, name, owner, params),
	m_iPin(params["pin"].GetInt())
{
	auto pullup = params.FindMember("pullup");
	m_fPullUp = pullup != params.MemberEnd() ? pullup->value.GetBool() : false;
}

void SensorDecoder::WriteConfig(dcclite::Packet &packet) const
{
	Decoder::WriteConfig(packet);

	packet.Write8(m_iPin);
	packet.Write8(
		(m_fPullUp ? dcclite::SensorDecoderFlags::SNRD_PULL_UP : 0)
	);
}
