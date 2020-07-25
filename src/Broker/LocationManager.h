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

#include <vector>

#include "Object.h"

#include <rapidjson/document.h>

class Decoder;
class Location;

class LocationManager: public dcclite::FolderObject
{	
	public:
		LocationManager(std::string name, const rapidjson::Value& params);
		~LocationManager() override
		{
			//empty
		}

		void RegisterDecoder(Decoder &decoder);
		void UnregisterDecoder(Decoder &decoder);

	private:
		std::vector<Location *> m_vecIndex;
};
