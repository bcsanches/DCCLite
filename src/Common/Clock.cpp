// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Clock.h"

namespace dcclite
{	
	Clock::Clock():
		m_StartTime(DefaultClock_t::now())		
	{
		m_CurrentTime = m_StartTime;
		m_PreviousTime = m_StartTime;
	}

	bool Clock::Tick(std::chrono::milliseconds min)
	{
		auto t = DefaultClock_t::now();

		if (t - m_CurrentTime < min)
			return false;

		m_PreviousTime = m_CurrentTime;
		m_CurrentTime = t;		

		return true;
	}

	
}
