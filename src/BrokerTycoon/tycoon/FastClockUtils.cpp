// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FastClockUtils.h"

#include "FastClock.h"

namespace dcclite::broker::tycoon::FastClockUtils
{
	std::chrono::local_time<std::chrono::system_clock::duration> GetLocalTime(FastClock::time_point time, const FastClock &fclock)
	{
		using namespace std::chrono;

		auto now = system_clock::now();

		auto local = zoned_time{ current_zone(), now };

		auto tp = local.get_local_time();

		return local.get_local_time() + (fclock.ConvertToRealTime(time - fclock.Now()));
	}
}

