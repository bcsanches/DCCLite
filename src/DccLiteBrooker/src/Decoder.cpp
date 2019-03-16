// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Decoder.h"

#include "Packet.h"
#include "Parser.h"

Decoder::Address::Address(const rapidjson::Value &value)
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

Decoder::Decoder(const Class &decoderClass, const Address &address, std::string name, DccLiteService &owner, const rapidjson::Value &params):
	Object(std::move(name)),
	m_iAddress(address),	
	m_rclManager(owner)
{
	//empty
}


void Decoder::WriteConfig(dcclite::Packet &packet) const
{
	packet.Write8(static_cast<std::uint8_t>(this->GetType()));
	m_iAddress.WriteConfig(packet);
}

void Decoder::Address::WriteConfig(dcclite::Packet &packet) const
{
	packet.Write16(m_iAddress);
}
