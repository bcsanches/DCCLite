// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Address.h"

#include <dcclite_shared/Packet.h>

#include <dcclite/Util.h>

namespace dcclite::broker::exec::dcc
{
	Address::Address(const rapidjson::Value &value):
		m_iAddress{ static_cast<uint16_t>(value.IsString() ? ParseNumber(value.GetString()) : value.GetInt()) }
	{		
		//empty
	}

	Address::Address(dcclite::Packet &packet) :
		m_iAddress(packet.Read<uint16_t>())
	{
		//empty
	}


	void Address::WriteConfig(dcclite::Packet &packet) const
	{
		packet.Write16(m_iAddress);
	}
}

