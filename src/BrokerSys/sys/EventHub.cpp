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
#include <memory>
#include <mutex>
#include <new>

#include <dcclite/Clock.h>
#include <dcclite/Log.h>

namespace dcclite::broker::sys
{
	class ObjectPool
	{
		public:
			explicit ObjectPool(uint32_t size):
				m_u32Size{ size },
				m_upPool{ new std::byte[size] }
			{
				this->Reset();
			}

			void *Alloc(size_t sz)
			{
				auto usedMem = this->GetUsedMem();
				if (usedMem + sz > m_u32Size)
				{
					dcclite::Log::Critical("[ObjectPool::Alloc] Not enough memory for extra {} bytes, throwing bad::alloc", sz);

					throw std::bad_alloc();
				}

#ifdef DEBUG
				++m_uAllocCount;
#endif

				auto *p = m_pBase;

				m_pBase += sz;

				return p;
			}

			void Reset() noexcept
			{
				m_pBase = m_upPool.get();

#ifdef DEBUG
				m_uAllocCount = 0;
#endif
			}

			const uint64_t GetUsedMem() const noexcept
			{
				return m_pBase - m_upPool.get();
			}

		private:
			uint32_t m_u32Size;

			std::byte *m_pBase;
			std::unique_ptr<std::byte[]> m_upPool;

#ifdef DEBUG
			uint32_t	m_uAllocCount = 0;
			uint32_t	m_uUsedMem = 0;
#endif

	};

	namespace EventHub
	{		
		class EventQueue
		{
			public:
				EventQueue() noexcept :
					m_pclHead{nullptr},
					m_pclLast{ nullptr }
				{
					//empty
				}

				EventQueue(const EventQueue &) = delete;

				EventQueue(EventQueue &&other) noexcept :
					m_pclHead{std::exchange(other.m_pclHead, nullptr)},
					m_pclLast{ std::exchange(other.m_pclLast, nullptr) }
				{
					//empty
				}

				EventQueue &operator=(const EventQueue &rhs) = delete;
				EventQueue &operator=(EventQueue &&rhs) noexcept
				{
					if (this != &rhs)
					{
						this->Clear();

						m_pclHead = std::exchange(rhs.m_pclHead, nullptr);
						m_pclLast = std::exchange(rhs.m_pclLast, nullptr);
					}

					return *this;
				}

				void PushBack(std::unique_ptr<IEvent> event) noexcept
				{	
					auto *p = event.release();

					assert(p);

					if (!m_pclHead)
					{
						m_pclHead = p;
						m_pclLast = p;
					}
					else
					{
						p->m_pclPrev = m_pclLast;
						m_pclLast->m_pclNext = p;
						m_pclLast = p;
					}										
				}

				void RemoveTarget(const IEventTarget &target) noexcept
				{
					auto *p = m_pclHead;

					while (p)
					{
						auto next = p->m_pclNext;
						if (&p->GetTarget() == &target)
						{
							this->RemoveNode(p);

							delete p;
						}

						p = next;
					}
				}

				void RemoveNode(IEvent *node) noexcept
				{
					assert(node);

					if (node->m_pclPrev == nullptr)
					{
						m_pclHead = node->m_pclNext;
					}
					else
					{
						node->m_pclPrev->m_pclNext = node->m_pclNext;						
					}

					if (node->m_pclNext == nullptr)
					{
						m_pclLast = node->m_pclPrev;
					}
					else
					{
						node->m_pclNext->m_pclPrev = node->m_pclPrev;						
					}

					node->m_pclPrev = nullptr;
					node->m_pclNext = nullptr;
				}

				~EventQueue()
				{
					//dcclite::Log::Debug("[EventQueue::~EventQueue] Goodbye");

					this->Clear();
				}

				bool IsEmpty() const noexcept
				{
					return m_pclHead == nullptr;
				}

				void FireTargets()
				{
					for (auto p = m_pclHead; p; p = p->m_pclNext)
						p->Fire();
				}

			private:
				void Clear() noexcept
				{
					while (m_pclHead)
					{
						auto next = m_pclHead->m_pclNext;

						delete m_pclHead;

						m_pclHead = next;
					}
				}

			private:
				IEvent *m_pclHead;
				IEvent *m_pclLast;
		};


		/// <summary>
		/// The only purpose of this structure is to make sure we a have a chance to clear the queue before the pools are destroyed 
		/// </summary>
		struct EventHubData
		{
			EventQueue				m_lstEventQueue;
			std::mutex				m_mtxEventQueueLock;
			std::condition_variable	m_clQueueMonitor;

			ObjectPool				m_arPools[2]{ ObjectPool{4096 * 8}, ObjectPool{4096 * 8} };

			int						m_iActivePool = 0;

			~EventHubData()
			{
				//clear the queue before the pools are destroyed
				m_lstEventQueue = EventQueue{};
			}
		};	

		static EventHubData g_sData;

#ifdef DCCLITE_EVENT_HUB_INTERNAL_POOL
		void *IEvent::operator new(size_t size)
		{			
			return g_sData.m_arPools[g_sData.m_iActivePool].Alloc(size);
		}

		void IEvent::operator delete(void *p)
		{
			//nothing todo			
		}
#endif

		namespace detail
		{
#ifdef DCCLITE_EVENT_HUB_INTERNAL_POOL
			void Lock()
			{
				g_sData.m_mtxEventQueueLock.lock();
			}

			void Unlock()
			{
				g_sData.m_mtxEventQueueLock.unlock();
			}

			void DoPostEvent(std::unique_ptr<IEvent> event)
			{				
				g_sData.m_lstEventQueue.PushBack(std::move(event));

				g_sData.m_mtxEventQueueLock.unlock();

				g_sData.m_clQueueMonitor.notify_one();
			}
#else

			void DoPostEvent(std::unique_ptr<IEvent> event)
			{
				{
					std::unique_lock<std::mutex> guard{ g_mtxEventQueueLock };

					g_lstEventQueue.PushBack(std::move(event));
				}

				g_clQueueMonitor.notify_one();	
			}

#endif

		}		

		void PumpEvents(const std::optional<Clock::DefaultClock_t::time_point> &timeoutTime)
		{
			EventQueue eventQueue;			

			{
				std::unique_lock<std::mutex> guard{ g_sData.m_mtxEventQueueLock };

				auto listLambda = [] { return !g_sData.m_lstEventQueue.IsEmpty(); };
				if (timeoutTime)
				{
					g_sData.m_clQueueMonitor.wait_until<dcclite::Clock::DefaultClock_t>(guard, timeoutTime.value(), listLambda);
				}
				else
				{
					g_sData.m_clQueueMonitor.wait(guard, listLambda);
				}									
								
				eventQueue = std::move(g_sData.m_lstEventQueue);
				
#ifdef DCCLITE_EVENT_HUB_INTERNAL_POOL
				g_sData.m_iActivePool = g_sData.m_iActivePool ? 0 : 1;
				g_sData.m_arPools[g_sData.m_iActivePool].Reset();
#endif
			}

			//
			//process events				
			eventQueue.FireTargets();			
		}

		void CancelEvents(const IEventTarget &target)
		{
			std::unique_lock<std::mutex> guard{ g_sData.m_mtxEventQueueLock };

			g_sData.m_lstEventQueue.RemoveTarget(target);
		}
	}	
}
