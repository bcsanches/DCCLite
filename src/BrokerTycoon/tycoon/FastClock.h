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
#include <dcclite/Object.h>

#include "sys/Thinker.h"

namespace dcclite::broker::tycoon
{
	class FastClock : public Object
	{
		public:
			FastClock(RName name, uint8_t rate);
			FastClock(const FastClock &) = delete;

			virtual const char *GetTypeName() const noexcept override
			{
				return "FastClock";
			}

			//
			//
			//chrono "interface"

			typedef std::chrono::seconds				duration;
			typedef duration::rep						rep;
			typedef duration::period					period;
			typedef std::chrono::time_point<FastClock>	time_point;
			static const bool is_steady = true;

			//
			//For thinkers
			typedef time_point TimePoint_t;

			inline time_point Now() const noexcept
			{
				return m_tElapsed;
			}

			void Start();
			void Stop();

			void SetRate(uint8_t rate);

		private:
			void OnTick(const dcclite::Clock::TimePoint_t tp);

		private:			
			dcclite::broker::sys::Thinker_t m_clThinker;

			time_point m_tElapsed;

			std::chrono::milliseconds m_tTickRate;

			uint8_t m_uRate;
	};
}
