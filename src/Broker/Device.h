// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Clock.h"
#include "Object.h"

#include <rapidjson/document.h>

class IDccLite_DeviceServices;
class Project;

class Device : public dcclite::FolderObject
{
	public:
		Device(std::string name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project);
		Device(std::string name, IDccLite_DeviceServices &dccService, const Project &project);	

		virtual void Update(const dcclite::Clock &clock) = 0;

	protected:
		IDccLite_DeviceServices &m_clDccService;
};

