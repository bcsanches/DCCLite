// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FastClockThinker.h"

namespace dcclite::broker::tycoon
{
	ThinkerManager::~ThinkerManager()
	{
		while (m_pclHead)
		{
			static_cast<FastClockThinker *>(m_pclHead)->Cancel();
		}
	}

	std::optional<FastClockDef::TimePoint_t> ThinkerManager::UpdateThinkers(const FastClockDef::TimePoint_t tp)
	{
		return FastClockThinker::UpdateThinkers(&m_pclHead, tp);
	}
}
