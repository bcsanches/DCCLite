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

void SignalDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	Decoder::Serialize(stream);



}