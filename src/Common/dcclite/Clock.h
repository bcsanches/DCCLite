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

namespace dcclite
{
	class Clock
	{
		public:
			typedef std::chrono::high_resolution_clock DefaultClock_t;
			typedef std::chrono::time_point<DefaultClock_t> TimePoint_t;

			Clock();

			bool Tick(std::chrono::milliseconds min = std::chrono::milliseconds{ 1 });

			inline std::chrono::milliseconds Delta() const
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_PreviousTime);
			}

			inline std::chrono::milliseconds Total() const
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_StartTime);
			}

			inline TimePoint_t Ticks() const noexcept
			{
				return m_CurrentTime;
			}

		private:						
			TimePoint_t m_StartTime;
			TimePoint_t m_CurrentTime;
			TimePoint_t m_PreviousTime;
	};
}
