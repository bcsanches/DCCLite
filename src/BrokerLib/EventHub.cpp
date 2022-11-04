// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "EventHub.h"

#include <condition_variable>
#include <list>
#include <mutex>

#include "Clock.h"

namespace dcclite::broker
{

	namespace EventHub
	{
		typedef std::list<std::unique_ptr<IEvent>> EventQueue_t;

		static EventQueue_t				m_lstEventQueue;
		static std::mutex				m_mtxEventQueueLock;
		static std::condition_variable	m_clQueueMonitor;

		void PostEvent(std::unique_ptr<IEvent> event)
		{
			{
				std::unique_lock<std::mutex> guard{ m_mtxEventQueueLock };

				m_lstEventQueue.push_back(std::move(event));
			}

			m_clQueueMonitor.notify_one();
		}

		void PumpEvents(const std::optional<Clock::DefaultClock_t::time_point> &timeoutTime)
		{
			EventQueue_t eventQueue;

			{
				std::unique_lock<std::mutex> guard{ m_mtxEventQueueLock };

				auto listLambda = [] { return !m_lstEventQueue.empty(); };
				if (timeoutTime)
				{
					m_clQueueMonitor.wait_until<dcclite::Clock::DefaultClock_t>(guard, timeoutTime.value(), listLambda);
				}
				else
				{
					m_clQueueMonitor.wait(guard, listLambda);
				}									
									
				std::swap(eventQueue, m_lstEventQueue);
			}

			//
			//process events				
			for (auto &item : eventQueue)
			{
				item->Fire();
			}
		}

		void CancelEvents(const IEventTarget &target)
		{
			std::unique_lock<std::mutex> guard{ m_mtxEventQueueLock };

			for (auto it = m_lstEventQueue.begin(); it != m_lstEventQueue.end();)
			{
				auto current = it++;

				if (&current->get()->GetTarget() == &target)
					m_lstEventQueue.erase(current);
			}
		}
	}
}
