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

#include <functional>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "Log.h"

namespace dcclite
{
	template <typename CLOCK>
	class BaseThinker
	{
		public:
			typedef std::function<void(const typename CLOCK::TimePoint_t)> Proc_t;

			typedef CLOCK Clock_t;
			typedef Clock_t::TimePoint_t TimePoint_t;

			template<typename CLASS, typename METHOD>
			static auto Binder(CLASS *obj, METHOD method) 
			{
				return [obj, method](const typename CLOCK::TimePoint_t tp) 
				{
					(obj->*method)(tp);
				};
			}

		public:
			BaseThinker(const std::string_view name, Proc_t proc) noexcept :
				m_pfnCallback{ proc },
				m_strvName{ name }
			{
				//empty
			}

			BaseThinker(BaseThinker &&) = delete;
			BaseThinker(const BaseThinker &) = delete;			

			BaseThinker &operator=(const BaseThinker &) = delete;
			BaseThinker &operator=(BaseThinker &&) = delete;

			virtual ~BaseThinker()
			{
				if (this->IsScheduled())
				{
					dcclite::Log::Critical("[BaseThinker] Destroying a scheduled thinker is not allowed, cancel it first. Thinker Name: {}", m_strvName);
					std::terminate();
				}
			}

			inline bool IsScheduled() const noexcept
			{
				return m_fScheduled;
			}

			[[nodiscard]] typename CLOCK::TimePoint_t GetTimePoint() const noexcept
			{				
				return m_tTimePoint;
			}

		protected:
			BaseThinker(BaseThinker **phead, const CLOCK::TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept :
				m_pfnCallback{ proc },
				m_strvName{ name }
			{
				this->Schedule(phead, tp);
			}

			//This seems to be evil.... or UB?
			BaseThinker(BaseThinker **phead, BaseThinker &&rhs) :
				m_pfnCallback{ rhs.m_pfnCallback },
				m_strvName{ rhs.m_strvName },
				m_tTimePoint{ rhs.m_tTimePoint },
				m_fScheduled{ rhs.m_fScheduled }
			{
				if (!m_fScheduled)
					return;

				m_pclNext = rhs.m_pclNext;
				m_pclPrev = rhs.m_pclPrev;

				if(m_pclNext)
					m_pclNext->m_pclPrev = this;

				if(m_pclPrev)
					m_pclPrev->m_pclNext = this;
				else
					*phead = this;

				rhs.m_fScheduled = false;
				rhs.m_pclNext = rhs.m_pclPrev = nullptr;
			}

			void Cancel(BaseThinker **phead) noexcept
			{
				if (!m_fScheduled)
					return;

				UnregisterThinker(phead, *this);

				m_fScheduled = false;
			}

			void Schedule(BaseThinker **phead, const CLOCK::TimePoint_t tp) noexcept
			{
				//
				//Sometimes they get scheduled multiple times...
				if ((m_fScheduled) && (tp == m_tTimePoint))
					return;

				this->Cancel(phead);

				m_tTimePoint = tp;

				RegisterThinker(phead, *this);

				m_fScheduled = true;
			}

			static std::optional<typename CLOCK::TimePoint_t> UpdateThinkers(BaseThinker **phead, const CLOCK::TimePoint_t tp)
			{
				while (*phead)
				{
					if ((*phead)->m_tTimePoint > tp)
					{
						return (*phead)->m_tTimePoint;
					}

					auto thinker = *phead;
					*phead = thinker->m_pclNext;

					//
					//We could do this in the loop before updating the pointers and avoid an if
					//but keep it here, so if the callback throws we are still in a consistent state
					//
					//If the callback throws we expect to abend, but, perhaps we change this someday  and to avoid a
					//painful time chasing a dangling pointer, lets play safe....
					if (*phead)
						(*phead)->m_pclPrev = nullptr;

					thinker->m_pclNext = nullptr;
					thinker->m_fScheduled = false;


					//dcclite::Log::Debug("[Thinker::UpdateThinkers] Running: {}", thinker->m_strvName);
					thinker->m_pfnCallback(tp);

					//
					//After the callback, does not touch the thinker anymore, it could be killed by the owner...
				}

				return std::nullopt;
			}

		private:
			static void RegisterThinker(BaseThinker **phead, BaseThinker &thinker) noexcept
			{
				auto **p = phead;
				BaseThinker *previous = nullptr;

				for (; (*p) && ((*p)->m_tTimePoint < thinker.m_tTimePoint); p = &(*p)->m_pclNext)
				{
					previous = *p;
				}

				thinker.m_pclNext = *p;

				if (*p)
					(*p)->m_pclPrev = &thinker;

				*p = &thinker;

				thinker.m_pclPrev = previous;			
			}

			static void UnregisterThinker(BaseThinker **phead, BaseThinker &thinker) noexcept
			{
				if (thinker.m_pclPrev == nullptr)
				{
					*phead = thinker.m_pclNext;
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
			}			

		private:
			Proc_t m_pfnCallback;

			CLOCK::TimePoint_t m_tTimePoint;

			const std::string_view m_strvName;

			BaseThinker *m_pclNext = nullptr;
			BaseThinker *m_pclPrev = nullptr;

			bool m_fScheduled = false;			
	};	
}
