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

#include "../sys/Service.h"

namespace dcclite::broker
{ 

	class DccppService: public Service
	{	
		public:
			DccppService(RName name, Broker &broker, const rapidjson::Value& params, const Project& project);
		
			~DccppService() override 
			{
				//empty
			}
		

			static std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &params, const Project &project);
	};
}
