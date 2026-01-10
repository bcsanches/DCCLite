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

			typedef std::chrono::seconds					duration;
			typedef duration::rep							rep;
			typedef duration::period						period;
			typedef std::chrono::time_point<FastClockDef>	time_point;
			static const bool is_steady = true;

			//
			//For thinkers
			typedef time_point TimePoint_t;		
	};
}
