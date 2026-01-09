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

#include <dcclite/BaseThinker.h>

#include <dcclite/Clock.h>

#define THINKER_MF_LAMBDA(proc) ([this](const dcclite::broker::sys::Thinker::TimePoint_t &tp) { this->proc(tp); })

namespace dcclite::broker::sys
{	
	class Thinker : public dcclite::BaseThinker<dcclite::Clock>
	{
		public:
			typedef dcclite::Clock::TimePoint_t TimePoint_t;
			typedef dcclite::BaseThinker<dcclite::Clock> Base_t;

			inline Thinker(const std::string_view name, Proc_t proc) noexcept :
				Base_t{ name, proc }
			{
				//empty
			}

			inline Thinker(const TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept :
				Base_t{ &g_pclThinkers, tp, name, proc }
			{
				//empty
			}

			~Thinker() override
			{
				//make sure we are unregistered
				this->Cancel();
			}

			inline void Schedule(const TimePoint_t tp) noexcept 
			{
				Base_t::Schedule(&g_pclThinkers, tp);
			}

			inline void Cancel() noexcept
			{
				Base_t::Cancel(&g_pclThinkers);
			}

			inline static std::optional<TimePoint_t> UpdateThinkers(const TimePoint_t tp)
			{
				return Base_t::UpdateThinkers(&g_pclThinkers, tp);
			}

		private:
			static Base_t *g_pclThinkers;
	};
}
