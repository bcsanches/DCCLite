// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "OutputDecoder.h"

#include <FmtUtils.h>
#include <Log.h>

#include "IDccLiteService.h"
#include "IDevice.h"

namespace dcclite::broker
{

	bool OutputDecoder::SetState(dcclite::DecoderStates newState, const char *requester)
	{
		if (m_kRequestedState != newState)
		{
			dcclite::Log::Info("[OutputDecoder::{}] [SetState] requested change from {} to {} by {}",
				this->GetName(),
				dcclite::DecoderStateName(m_kRequestedState),
				dcclite::DecoderStateName(newState),
				requester
			);

			m_kRequestedState = newState;

			//Allow manager to know it and allow it to propagate changes
			m_rclManager.Decoder_OnStateChanged(*this);

			//device will take care of sending this down the network if necessary
			m_rclDevice.Decoder_OnChangeStateRequest(*this);

			return true;
		}

		return false;
	}

	void OutputDecoder::Serialize(dcclite::JsonOutputStream_t& stream) const
	{
		RemoteDecoder::Serialize(stream);

		stream.AddBool("requestedState", m_kRequestedState == dcclite::DecoderStates::ACTIVE);
	}
}
