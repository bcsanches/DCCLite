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

#include <dcclite/Log.h>

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
#if 0
		auto **p = &g_pclThinkers;		

		for (; (*p) && ((*p)->m_tTimePoint < thinker.m_tTimePoint); p = &(*p)->m_pclNext);

		thinker.m_pclNext = *p;
		*p = &thinker;	

#else		

		auto **p = &g_pclThinkers;		
		Thinker *previous = nullptr;

		for (; (*p) && ((*p)->m_tTimePoint < thinker.m_tTimePoint); p = &(*p)->m_pclNext)
		{
			previous = *p;			
		}
			

		thinker.m_pclNext = *p;

		if (*p)
			(*p)->m_pclPrev = &thinker;

		*p = &thinker;

		thinker.m_pclPrev = previous;		
#endif
	}

	void Thinker::UnregisterThinker(Thinker &thinker) noexcept
	{
#if 0
		auto **p = &g_pclThinkers;
		
		while(*p)
		{
			if (*p == &thinker)
			{
				*p = thinker.m_pclNext;
				break;
			}

			p = &(*p)->m_pclNext;
		}

#else

		if (thinker.m_pclPrev == nullptr)
		{
			g_pclThinkers = thinker.m_pclNext;
		}
		else
		{
			thinker.m_pclPrev->m_pclNext = thinker.m_pclNext;
		}		

		if (thinker.m_pclNext)
		{
			thinker.m_pclNext->m_pclPrev = thinker.m_pclPrev;			
		}

		thinker.m_pclNext = nullptr;
		thinker.m_pclPrev = nullptr;

#endif
	}

	Thinker::Thinker(const std::string_view name, Proc_t proc) noexcept:
		m_strvName{name},
		m_pfnCallback{proc}
	{

	}

	Thinker::Thinker(const dcclite::Clock::TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept:
		m_strvName{ name },
		m_pfnCallback{ proc }
	{
		this->Schedule(tp);
	}	

	Thinker::~Thinker() noexcept
	{
		this->Cancel();
	}

	void Thinker::Schedule(dcclite::Clock::TimePoint_t tp) noexcept
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

			//
			//We could do this in the loop before updating the pointers and avoid an if
			//but keep it here, so if the callback throws we are still in a consistent state
			//
			//If the callback throws we expect to abend, but, perhaps we change this someday  and to avoid a
			//painful time chasing a dangling pointer, lets play safe....
			if (g_pclThinkers)
				g_pclThinkers->m_pclPrev = nullptr;


			thinker->m_pclNext = nullptr;			
			thinker->m_fScheduled = false;


			//dcclite::Log::Debug("[Thinker::UpdateThinkers] Running: {}", thinker->m_strvName);
			thinker->m_pfnCallback(tp);

			//
			//After the callback, does not touch the thinker anymore, it could be killed by the owner...
		}

		return std::nullopt;
	}
}
