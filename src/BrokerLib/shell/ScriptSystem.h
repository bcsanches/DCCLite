// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <string_view>

#include <sol/sol.hpp>

namespace dcclite::broker
{	
	class Broker;
}

namespace dcclite::broker::ScriptSystem
{
	class IScriptSupport
	{
		public:
			virtual void IScriptSupport_RegisterProxy(sol::table &table) = 0;

			virtual void IScriptSupport_OnVMInit(sol::state &state) = 0;
			virtual void IScriptSupport_OnVMFinalize(sol::state &state) = 0;
	};	

	extern void Start(Broker &broker);
	extern void Stop();	
}
