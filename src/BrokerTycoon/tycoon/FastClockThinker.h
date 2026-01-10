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

#include "FastClockDefs.h"

#include <dcclite/BaseThinker.h>

namespace dcclite::broker::tycoon
{	
	class FastClockThinker;

	class ThinkerManager
	{
		public:
			ThinkerManager() = default;

			ThinkerManager(const ThinkerManager &) = delete;
			ThinkerManager &operator=(const ThinkerManager &) = delete;

			ThinkerManager(ThinkerManager &&) = delete;
			ThinkerManager &operator=(ThinkerManager &&) = delete;

			~ThinkerManager();

			std::optional<FastClockDef::TimePoint_t> UpdateThinkers(const FastClockDef::TimePoint_t tp);

		private:
			friend class FastClockThinker;

			dcclite::BaseThinker<FastClockDef> *m_pclHead = nullptr;
	};

	class FastClockThinker : public dcclite::BaseThinker<FastClockDef>
	{
		public:
			typedef dcclite::BaseThinker<FastClockDef> Base_t;
			inline FastClockThinker(ThinkerManager &manager, const std::string_view name, Proc_t proc) noexcept :
				Base_t{ name, proc },
				m_rclManager{ manager }
			{
				//empty
			}
			inline FastClockThinker(ThinkerManager &manager, const TimePoint_t tp, const std::string_view name, Proc_t proc) noexcept :
				Base_t{ &manager.m_pclHead, tp, name, proc },
				m_rclManager{ manager }
			{
				//empty
			}

			inline FastClockThinker(FastClockThinker &&rhs) noexcept:
				Base_t{ &rhs.m_rclManager.m_pclHead, std::move(rhs) },
				m_rclManager{ rhs.m_rclManager }
			{
				//empty
			}

			FastClockThinker(FastClockThinker const &) = delete;

			~FastClockThinker() override
			{
				//make sure we are unregistered
				this->Cancel();
			}

			inline void Schedule(const TimePoint_t tp) noexcept
			{
				Base_t::Schedule(&m_rclManager.m_pclHead, tp);
			}

			inline void Cancel() noexcept
			{
				Base_t::Cancel(&m_rclManager.m_pclHead);
			}

			inline static std::optional<FastClockThinker::TimePoint_t> UpdateThinkers(Base_t **head, const FastClockThinker::TimePoint_t tp)
			{
				return Base_t::UpdateThinkers(head, tp);
			}

		private:
			ThinkerManager &m_rclManager;
	};
}
