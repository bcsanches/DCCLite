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
#include <memory>
#include <optional>

#include <Clock.h>

namespace dcclite::broker
{
	namespace Messenger
	{
		class IEventTarget
		{

		};

		class IEvent
		{
		public:
			IEvent(IEventTarget &target):
				m_rclTarget(target)
			{
				//empty
			}

			virtual void Fire() = 0;

			inline IEventTarget &GetTarget() noexcept
			{
				return m_rclTarget;
			}

		private:
			IEventTarget &m_rclTarget;
		};

		void PostEvent(std::unique_ptr<IEvent> event);
		void PumpEvents(const std::optional<Clock::DefaultClock_t::time_point> &timeoutTime);

		void CancelEvents(const IEventTarget &target);

		template <typename T, typename... Args>
		void MakeEvent(Args ...args)
		{
			PostEvent(std::make_unique<T>(args...));
		}
	}
}