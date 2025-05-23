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

#include <dcclite_shared/BitPack.h>

#include "sys/Service.h"

#include "ILoconetSlot.h"

namespace dcclite::broker::shell::ln
{ 
	class LoconetService: public sys::Service
	{	
		public:
			static const char *TYPE_NAME;

			static void RegisterFactory();

			LoconetService(RName name, sys::Broker &broker, const rapidjson::Value& params);
		
			~LoconetService() override
			{
				//empty
			}

			const char *GetTypeName() const noexcept override
			{
				return "LoconetService";
			}
	};
}
