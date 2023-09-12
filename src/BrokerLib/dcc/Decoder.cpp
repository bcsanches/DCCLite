// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <Log.h>

#include "Decoder.h"

#include "IDccLiteService.h"
#include "IDevice.h"
#include "Parser.h"
#include "Packet.h"

namespace dcclite::broker
{	
	Decoder::Decoder(const DccAddress &address, RName name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params) :
		Object{name},
		m_iAddress(address),
		m_rclManager(owner),
		m_rclDevice(dev)
	{
		auto it = params.FindMember("location");
		if (it != params.MemberEnd())
			m_rnLocationHint = RName{ it->value.GetString() };
	}

	void Decoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Object::Serialize(stream);

		stream.AddIntValue("address", m_iAddress.GetAddress());
		stream.AddStringValue("deviceName", m_rclDevice.GetDeviceName().GetData());
		stream.AddStringValue("systemName", m_rclManager.Decoder_GetSystemName().GetData());

		if (m_rnLocationHint)
			stream.AddStringValue("locationHint", m_rnLocationHint.GetData());
	}
}

