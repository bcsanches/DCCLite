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

#include "DispatcherService.h"

#include <sol/sol.hpp>

namespace dcclite::broker::shell::dispatcher::detail
{	

	class DispatcherServiceScripter: public DispatcherService
	{
		protected:
			inline DispatcherServiceScripter(RName name, sys::Broker &broker, const rapidjson::Value &params):
				DispatcherService(name, broker, params)
			{
				//empty
			}

		public:
			void IScriptSupport_OnVMInit(sol::state &sol);			
			void IScriptSupport_RegisterProxy(sol::state &sol, sol::table &table);
	};	
}
