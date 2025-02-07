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

#include "Log.h"

#define DCCLITE_EVENT_HUB_INTERNAL_POOL

namespace dcclite::broker
{
	namespace EventHub
	{
		class IEventTarget
		{
			public:
				virtual ~IEventTarget() = default;

			protected:
				IEventTarget() = default;
				IEventTarget(const IEventTarget &rhs) = delete;
				IEventTarget(IEventTarget &&other) = delete;
		};

		class IEvent
		{
			public:
				IEvent(IEventTarget &target) :
					m_rclTarget(target)
				{
					//empty
				}

				virtual ~IEvent() = default;

				virtual void Fire() = 0;

				inline IEventTarget &GetTarget() noexcept
				{
					return m_rclTarget;
				}

#ifdef DCCLITE_EVENT_HUB_INTERNAL_POOL
				void *operator new(size_t size);
				void operator delete(void *p);
#endif

			private:
				IEventTarget &m_rclTarget;

				IEvent *m_pclNext = nullptr;
				IEvent *m_pclPrev = nullptr;

				friend class EventQueue;
		};

		namespace detail
		{
			void DoPostEvent(std::unique_ptr<IEvent> event);

			void Lock();
			void Unlock();
		}
		void PumpEvents(const std::optional<Clock::DefaultClock_t::time_point> &timeoutTime);

		void CancelEvents(const IEventTarget &target);

#ifdef DCCLITE_EVENT_HUB_INTERNAL_POOL
		template <typename T, typename... Args>
		void PostEvent(Args ...args)
		{
			//
			//we must lock before creating the event, because it will use the pool and we must guarantee that the pool will stay until the event is posted
			detail::Lock();

			std::unique_ptr<IEvent> ptr;

			try
			{
				ptr = std::make_unique<T>(std::forward<Args>(args)...);
			}
#ifdef DCCLITE_DEBUG
			catch (std::bad_alloc &)
			{
				dcclite::Log::Warn("[EventHub::PostEvent] Alloc failed, are you debugging??");

				detail::Unlock();

				//ignore it.. drop the event...
				return;
			}
#endif
			catch (...)
			{
				detail::Unlock();

				throw;
			}

			//It will take care of unlocking the system...
			detail::DoPostEvent(std::move(ptr));
		}
#else
		template <typename T, typename... Args>
		void PostEvent(Args ...args)
		{
			detail::DoPostEvent(std::make_unique<T>(std::forward<Args>(args)...));
		}
#endif

	}
}
