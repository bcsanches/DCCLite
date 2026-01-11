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

#include <dcclite/BaseThinker.h>
#include <dcclite/Clock.h>
#include <dcclite/Object.h>

#include <sys/Thinker.h>

#include "FastClockDefs.h"
#include "FastClockThinker.h"

namespace dcclite::broker::tycoon
{
	class FastClock : public Object
	{
		public:
			typedef FastClockDef::time_point time_point;
			typedef uint16_t Rate_t;

			FastClock(RName name, Rate_t rate);
			FastClock(const FastClock &) = delete;

			virtual const char *GetTypeName() const noexcept override
			{
				return "FastClock";
			}			

			inline time_point Now() const noexcept
			{
				return m_tElapsed;
			}

			void Start();
			void Stop();

			void SetRate(Rate_t rate);

			inline FastClockThinker MakeThinker(const std::string_view name, FastClockThinker::Proc_t proc)
			{
				return FastClockThinker{ m_clThinkerManager, {}, name, proc };
			}

			inline FastClockThinker MakeThinker(time_point tp, const std::string_view name, FastClockThinker::Proc_t proc)
			{
				return FastClockThinker{ m_clThinkerManager, tp, name, proc };				
			}

			inline std::chrono::seconds ConvertToRealTime(std::chrono::seconds seconds) const noexcept
			{
				return seconds / m_uRate;
			}

		private:
			void OnTick(const dcclite::Clock::TimePoint_t tp);

		private:			
			dcclite::broker::sys::Thinker m_clThinker;

			ThinkerManager m_clThinkerManager;

			time_point m_tElapsed;

			std::chrono::milliseconds m_tTickRate;

			Rate_t m_uRate;
	};	
}
