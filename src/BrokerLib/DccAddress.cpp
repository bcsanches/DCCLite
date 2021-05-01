// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DccAddress.h"

#include "Parser.h"
#include "Packet.h"

namespace dcclite::broker
{
	DccAddress::DccAddress(const rapidjson::Value &value)
	{
		if (value.IsString())
		{
			dcclite::Parser parser{ value.GetString() };

			int adr;
			if (parser.GetNumber(adr) != dcclite::Tokens::NUMBER)
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

	DccAddress::DccAddress(dcclite::Packet &packet) :
		m_iAddress(packet.Read<uint16_t>())
	{
		//empty
	}


	void DccAddress::WriteConfig(dcclite::Packet &packet) const
	{
		packet.Write16(m_iAddress);
	}
}

