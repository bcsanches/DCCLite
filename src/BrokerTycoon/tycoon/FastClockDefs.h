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
			using period = std::milli;								// 1 ms per tick
			using duration = std::chrono::duration<rep, period>;	//seconds

			using time_point = std::chrono::time_point<FastClockDef>;
						
			static const bool is_steady = true;

			//
			//For thinkers
			typedef time_point TimePoint_t;

			static inline uint64_t ConvertToIntMs(const time_point tp) noexcept
			{
				static_assert(std::chrono::duration_cast<std::chrono::milliseconds>(duration{ 1 }).count() == 1, "FastClockDef duration must be in milliseconds");

				return tp.time_since_epoch().count();
			}

			static inline time_point ConvertFromIntMs(const uint64_t value) noexcept
			{
				static_assert(std::chrono::duration_cast<std::chrono::milliseconds>(duration{ 1 }).count() == 1, "FastClockDef duration must be in milliseconds");

				return time_point{ duration{ value } };
			}
	};
}
