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

#include "SignalDecoder.h"

#include <fmt/format.h>

namespace dcclite::broker
{

	VirtualDevice::VirtualDevice(std::string name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project) :
		Device{ std::move(name), dccService, params, project }
	{
		this->Load();
	}

	VirtualDevice::VirtualDevice(std::string name, IDccLite_DeviceServices &dccService, const Project &project) :
		Device{ std::move(name), dccService, project }
	{
		//emtpy
	}	

	void VirtualDevice::CheckLoadedDecoder(Decoder &decoder)
	{
		if (!dynamic_cast<SignalDecoder *>(&decoder))
			throw std::invalid_argument(fmt::format("[VirtualDevice::CheckLoadedDecoder] Decoder {} must be a SignalDecoder subtype, but it is: {}", decoder.GetName(), decoder.GetTypeName()));
	}
}
