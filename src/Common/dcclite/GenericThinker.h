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
#include <string_view>

namespace dcclite
{
	template <typename CLOCK>
	class Thinker
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
			Thinker(const std::string_view name, Proc_t proc) noexcept :
				m_pfnCallback{ proc },
				m_strvName{ name }
			{
				//empty
			}

			Thinker(const CLOCK::TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept :
				m_pfnCallback{ proc },
				m_strvName{ name }
			{
				this->Schedule(tp);
			}

			Thinker(Thinker &&) = delete;
			Thinker(const Thinker &) = delete;

			Thinker &operator=(const Thinker &) = delete;
			Thinker &operator=(Thinker &&) = delete;

			~Thinker() noexcept
			{
				this->Cancel();
			}

			void Schedule(const CLOCK::TimePoint_t tp) noexcept
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

			void Cancel() noexcept
			{
				if (!m_fScheduled)
					return;

				UnregisterThinker(*this);

				m_fScheduled = false;
			}

			inline bool IsScheduled() const noexcept
			{
				return m_fScheduled;
			}

			static std::optional<typename CLOCK::TimePoint_t> UpdateThinkers(const CLOCK::TimePoint_t tp)
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

		private:
			static void RegisterThinker(Thinker &thinker) noexcept
			{
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
			}

			static void UnregisterThinker(Thinker &thinker) noexcept
			{
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
			}			

		private:
			Proc_t m_pfnCallback;

			CLOCK::TimePoint_t m_tTimePoint;

			const std::string_view m_strvName;

			Thinker *m_pclNext = nullptr;
			Thinker *m_pclPrev = nullptr;

			bool m_fScheduled = false;

			static Thinker *g_pclThinkers;
	};

	template <typename CLOCK>
	Thinker<CLOCK> *Thinker<CLOCK>::g_pclThinkers = nullptr;
}
