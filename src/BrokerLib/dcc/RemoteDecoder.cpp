// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "RemoteDecoder.h"

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>
#include <dcclite/JsonUtils.h>

#include "IDccLiteService.h"
#include "Packet.h"

namespace dcclite::broker
{
	RemoteDecoder::RemoteDecoder(const DccAddress &address, RName name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params):
		StateDecoder(address, name, owner, dev, params)	
	{	
		m_fBroken = dcclite::json::TryGetDefaultBool(params, "broken", false);		
	}

	void RemoteDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		packet.Write8(static_cast<std::uint8_t>(this->GetType()));
		this->GetAddress().WriteConfig(packet);	
	}

	bool RemoteDecoder::SyncRemoteState(dcclite::DecoderStates state)
	{
		if(this->SetState(state, !m_fBroken))
		{			
			dcclite::Log::Info("[RemoteDecoder::{}] [SyncRemoteState] changed: {}", this->GetName(), dcclite::DecoderStateName(state));

			//If it is broken, dont publish state change, probably is garbage
			//
			//It is not worth to add a special case to the network sync code, so we simple ignore it here
			if (!m_fBroken)
			{
				m_sigRemoteStateSync(*this);				
			}
			
			return true;
		}	

		return false;
	}

	void RemoteDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		StateDecoder::Serialize(stream);
			
		stream.AddBool("broken", m_fBroken);
	}

}