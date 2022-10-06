// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "IDccLiteService.h"
#include "RemoteDecoder.h"

#include "Log.h"
#include "Packet.h"

namespace dcclite::broker
{


	RemoteDecoder::RemoteDecoder(const DccAddress &address, std::string name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params):
		Decoder(address, std::move(name), owner, dev, params)	
	{	
		auto it = params.FindMember("broken");
		if (it != params.MemberEnd())
			m_fBroken = it->value.GetBool();
	}

	void RemoteDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		packet.Write8(static_cast<std::uint8_t>(this->GetType()));
		this->GetAddress().WriteConfig(packet);	
	}

	void RemoteDecoder::SyncRemoteState(dcclite::DecoderStates state)
	{
		if((state != m_kRemoteState) && !m_fBroken)
		{
			m_kRemoteState = state;

			dcclite::Log::Info("[{}::SyncRemoteState] changed: {}", this->GetName(), dcclite::DecoderStateName(state));

			m_sigRemoteStateSync(*this);
			m_rclManager.Decoder_OnStateChanged(*this);
		}	
	}

	void RemoteDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Decoder::Serialize(stream);
	
		stream.AddBool("remoteActive", m_kRemoteState == dcclite::DecoderStates::ACTIVE);	
		stream.AddBool("broken", m_fBroken);
	}

}