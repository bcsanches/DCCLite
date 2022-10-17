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

#include "Clock.h"

#define THINKER_MF_LAMBDA(proc) ([this](const dcclite::Clock::TimePoint_t &tp) {this->proc(tp); })

namespace dcclite::broker
{
	class Thinker
	{
		typedef std::function<void(const dcclite::Clock::TimePoint_t)> Proc_t;

		public:
			Thinker(const std::string_view name, Proc_t proc) noexcept;
			Thinker(const dcclite::Clock::TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept;

			Thinker(Thinker &&) = delete;
			Thinker(const Thinker &) = delete;

			Thinker &operator=(const Thinker &) = delete;
			Thinker &operator=(Thinker &&) = delete;
			
			~Thinker() noexcept;

			void SetNext(const dcclite::Clock::TimePoint_t tp) noexcept;
			void Cancel() noexcept;

			inline bool IsScheduled() const noexcept
			{
				return m_fScheduled;
			}
			
			static std::optional<dcclite::Clock::TimePoint_t> UpdateThinkers(const dcclite::Clock::TimePoint_t tp);

		private:
			static void RegisterThinker(Thinker &thinker) noexcept;
			static void UnregisterThinker(Thinker &thinker) noexcept;

			friend bool ThinkerPointerComparer(const Thinker *a, const Thinker *b) noexcept;

		private:
			Proc_t m_pfnCallback;

			dcclite::Clock::TimePoint_t m_tTimePoint;

			const std::string_view m_strvName;

			Thinker *m_pclNext = nullptr;

			bool m_fScheduled = false;
	};
}
