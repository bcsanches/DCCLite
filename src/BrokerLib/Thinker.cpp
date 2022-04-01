// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Thinker.h"

#include <assert.h>

#include "Log.h"

namespace dcclite::broker
{
#if 0
	inline bool ThinkerPointerComparer(const Thinker *a, const Thinker *b) noexcept
	{
		return a->m_tTimePoint < b->m_tTimePoint;
	}

	static std::priority_queue < Thinker *, std::vector<Thinker *>, decltype(ThinkerPointerComparer)> m_qQueue(ThinkerPointerComparer);	
#endif

	static Thinker *g_pclThinkers = nullptr;

	void Thinker::RegisterThinker(Thinker &thinker) noexcept
	{					
		auto **p = &g_pclThinkers;

		for (; (*p) && ((*p)->m_tTimePoint < thinker.m_tTimePoint); p = &(*p)->m_pclNext);

		thinker.m_pclNext = *p;
		*p = &thinker;
	}

	void Thinker::UnregisterThinker(Thinker &thinker) noexcept
	{
		if (&thinker == g_pclThinkers)
		{
			g_pclThinkers = thinker.m_pclNext;

			return;
		}

		auto prev = g_pclThinkers;
		auto node = g_pclThinkers->m_pclNext;
		
		do
		{
			if (node == &thinker)
			{
				prev->m_pclNext = node->m_pclNext;

				break;
			}
				
			prev = node;
			node = node->m_pclNext;
		} while (node);		
	}

	Thinker::Thinker(Proc_t proc) noexcept:
		m_pfnCallback{proc}
	{

	}

	Thinker::Thinker(const dcclite::Clock::TimePoint_t tp, Proc_t proc) noexcept:
		m_pfnCallback{ proc }
	{
		this->SetNext(tp);
	}	

	Thinker::~Thinker() noexcept
	{
		this->Cancel();
	}

	void Thinker::SetNext(dcclite::Clock::TimePoint_t tp) noexcept
	{
		//
		//Sometimes they get scheduled multiple times...
		if ((m_fScheduled) && (tp == m_tTimePoint))
			return;

		this->Cancel();

		m_tTimePoint = tp;

		RegisterThinker(*this);

		m_fScheduled = true;
	}

	void Thinker::Cancel() noexcept
	{
		if (!m_fScheduled)
			return;

		UnregisterThinker(*this);

		m_fScheduled = false;
	}

	std::optional<dcclite::Clock::TimePoint_t> Thinker::UpdateThinkers(const dcclite::Clock::TimePoint_t tp)
	{		
		while (g_pclThinkers)
		{
			if (g_pclThinkers->m_tTimePoint > tp)
			{				
				return g_pclThinkers->m_tTimePoint;
			}				

			auto thinker = g_pclThinkers;
			g_pclThinkers = thinker->m_pclNext;
			thinker->m_pclNext = nullptr;
			thinker->m_fScheduled = false;

			thinker->m_pfnCallback(tp);
		}

		return std::nullopt;
	}
}