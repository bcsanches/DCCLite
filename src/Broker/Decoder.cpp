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

DccAddress::DccAddress(const rapidjson::Value &value)
{
	if (value.IsString())
	{		
		dcclite::Parser parser{ value.GetString() };
		
		int adr;		
		if (parser.GetNumber(adr) != dcclite::TOKEN_NUMBER)
		{					
			throw std::runtime_error(fmt::format("error: Decoder::Address::Address(const nlohmann::json::value_type &value) invalid value for address, see {}", value.GetString()));
		}		

		m_iAddress = adr;
	}
	else
	{
		m_iAddress = value.GetInt();
	}
}

Decoder::Decoder(const Class &decoderClass, const DccAddress &address, std::string name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params):
	Object(std::move(name)),
	m_iAddress(address),	
	m_rclManager(owner),
	m_rclDevice(dev)
{	
	auto it = params.FindMember("location");
	if(it != params.MemberEnd())
		m_strLocationHint = it->value.GetString();
}

void DccAddress::WriteConfig(dcclite::Packet &packet) const
{
	packet.Write16(m_iAddress);
}

void Decoder::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	Object::Serialize(stream);

	stream.AddIntValue("address", m_iAddress.GetAddress());	
	stream.AddStringValue("deviceName", m_rclDevice.GetDeviceName());	

	if(!m_strLocationHint.empty())
		stream.AddStringValue("locationHint", m_strLocationHint);
}

