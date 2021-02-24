// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SignalDecoder.h"

static Decoder::Class signalDecoderClass("VirtualSignal",
	[](const Decoder::Class &decoderClass, const DccAddress &address, const std::string &name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params)
	-> std::unique_ptr<Decoder> { return std::make_unique<SignalDecoder>(decoderClass, address, name, owner, dev, params); }
);


SignalDecoder::SignalDecoder(
	const Class &decoderClass,
	const DccAddress &address,
	const std::string &name,
	IDccLite_DecoderServices &owner,
	IDevice_DecoderServices &dev,
	const rapidjson::Value &params
) :
	Decoder(decoderClass, address, name, owner, dev, params)
{
	auto headsData = params.FindMember("heads");
	if ((headsData == params.MemberEnd()) || (!headsData->value.IsObject()))
	{
		throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: expected heads object for {}", this->GetName()));		
	}

	for (auto &headElement : headsData->value.GetObject())
	{
		m_setHeads.insert(std::make_pair(headElement.name.GetString(), headElement.value.GetString()));
	}


}


void SignalDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	Decoder::Serialize(stream);



}