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

#include "Service.h"


namespace dcclite::broker
{ 
	constexpr auto INFO_SERVICE_NAME = "InfoService";

	class InfoService: public Service
	{	
		public:
			InfoService(RName name, Broker &broker, const Project& project);
		
			~InfoService() override
			{
				//empty
			}								

			const char *GetTypeName() const noexcept override
			{
				return "InfoService";
			}
	};
}
