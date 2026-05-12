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

#include <chrono>

#include <dcclite/Clock.h>

namespace dcclite::broker::tycoon
{
	class FastClockDef
	{
		public:		
			//
			//
			//chrono "interface"
			using rep = int64_t;
			using period = std::ratio<1>;							// 1 second per tick
			using duration = std::chrono::duration<rep, period>;	//seconds

			using time_point = std::chrono::time_point<FastClockDef>;
						
			static const bool is_steady = true;

			//
			//For thinkers
			typedef time_point TimePoint_t;		
	};
}
