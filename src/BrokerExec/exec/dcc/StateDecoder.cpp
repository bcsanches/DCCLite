// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "StateDecoder.h"

#include "IDccLiteService.h"

namespace dcclite::broker::exec::dcc
{
	StateDecoder::StateDecoder(const Address &address, RName name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params):
		Decoder(address, std::move(name), owner, dev, params)	
	{	
		//empty
	}

	void StateDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Decoder::Serialize(stream);
	
		stream.AddBool("active", m_kState == dcclite::DecoderStates::ACTIVE);			
	}

	bool StateDecoder::SetState(dcclite::DecoderStates state, bool publishUpdate)
	{
		if (state == m_kState)
			return false;

		m_kState = state;
		
		if(publishUpdate)
			m_rclManager.Decoder_OnStateChanged(*this);

		return true;
	}
}
