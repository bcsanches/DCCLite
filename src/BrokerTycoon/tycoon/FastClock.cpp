// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FastClock.h"

#include <dcclite/Log.h>

namespace dcclite::broker::tycoon
{
	FastClock::FastClock(RName name, Rate_t rate) :
		Object{ name },
		m_clThinker{ "FastClock::OnTick", THINKER_MF_LAMBDA(OnTick) }
	{
		this->SetRate(rate);
	}

	void FastClock::SetRate(Rate_t rate)
	{
		if (rate == 0)
		{
			throw std::invalid_argument("[Tycoon::FastClock::FastClock] rate must be greater than zero");
		}

		if (60 % rate)
		{
			//we will have a small error accumulation over time if certain cases, but for our purposes it is acceptable, but warn user
			dcclite::Log::Warn("[Tycoon::FastClock::FastClock] rate {} does not divide evenly into 60, small timing errors may accumulate over time", rate);
		}

		using namespace std::chrono;
		using namespace std::chrono_literals;

		m_tTickRate = 60000ms / rate;
		m_uRate = rate;
	}

	void FastClock::Start()
	{
		if (m_clThinker.IsScheduled())
			return;

		m_clThinker.Schedule(dcclite::Clock::DefaultClock_t::now() + m_tTickRate);
	}

	void FastClock::Stop()
	{
		m_clThinker.Cancel();
	}

	void FastClock::OnTick(const dcclite::Clock::TimePoint_t tp)
	{
		m_clThinker.Schedule(dcclite::Clock::DefaultClock_t::now() + m_tTickRate);

		using namespace std::chrono_literals;
		m_tElapsed += 1min;

		//dcclite::Log::Trace("[FastClock::OnTick] Tick. Elapsed time: {}s", std::chrono::duration_cast<std::chrono::seconds>(m_tElapsed.time_since_epoch()).count());
		m_clThinkerManager.UpdateThinkers(m_tElapsed);
	}
}
