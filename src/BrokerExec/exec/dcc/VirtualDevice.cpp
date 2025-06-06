// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "VirtualDevice.h"

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>

#include "SignalDecoder.h"
#include "VirtualTurnoutDecoder.h"
#include "VirtualSensorDecoder.h"

namespace dcclite::broker::exec::dcc
{

	VirtualDevice::VirtualDevice(RName name, sys::Broker &broker, IDccLite_DeviceServices &dccService, const rapidjson::Value &params) :
		Device{ name, broker, dccService, params }
	{
		this->Load();
	}

	VirtualDevice::VirtualDevice(RName name, IDccLite_DeviceServices &dccService) :
		Device{ name, dccService }
	{
		//emtpy
	}	

	void VirtualDevice::CheckIfDecoderTypeIsAllowed(Decoder &decoder)
	{		
		if (dynamic_cast<SignalDecoder *>(&decoder))
			return;

		if (dynamic_cast<VirtualTurnoutDecoder *>(&decoder))
			return;

		if (dynamic_cast<VirtualSensorDecoder *>(&decoder))
			return;

		throw std::invalid_argument(fmt::format("[VirtualDevice::{}] [CheckLoadedDecoder] Decoder {} must be a SignalDecoder, VirtualSensorDecoder or VirtualTurnoutDecoder subtype, but it is: {}", this->GetName(), decoder.GetName(), decoder.GetTypeName()));
	}

	bool VirtualDevice::IsInternalDecoderAllowed() const noexcept
	{
		return true;
	}

	void VirtualDevice::Decoder_OnChangeStateRequest(const Decoder &decoder) noexcept
	{
		if (auto t = dynamic_cast<const VirtualTurnoutDecoder *>(&decoder))
		{
			//sorry kids...
			auto turnout = const_cast<VirtualTurnoutDecoder *>(t);

			turnout->SyncRemoteState(turnout->GetRequestedState());
		}
	}
}
