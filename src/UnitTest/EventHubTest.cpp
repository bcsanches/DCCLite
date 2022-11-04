// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include "EventHub.h"

class EventTargetMockup : public dcclite::broker::EventHub::IEventTarget
{

};

class MyTestEvent : public dcclite::broker::EventHub::IEvent
{
	public:
		MyTestEvent(std::function<void()> lambda) :
			IEvent{ m_clMockup },
			m_pfnLambda{ lambda }
		{
			++g_iObjectCount;
		}

		virtual ~MyTestEvent()
		{
			--g_iObjectCount;
		}

		void Fire() override
		{
			m_pfnLambda();
		}

		static int GetObjectCount()
		{
			return g_iObjectCount;
		}

	private:
		EventTargetMockup m_clMockup;
		std::function<void()> m_pfnLambda;

		static int g_iObjectCount;
};

int MyTestEvent::g_iObjectCount = 0;

TEST(EventHub, Basic)
{					
	bool called = false;

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	dcclite::broker::EventHub::MakeEvent<MyTestEvent>([&called] { called = true; });

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 1);
	ASSERT_FALSE(called);

	dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
	ASSERT_TRUE(called);
}

TEST(EventHub, CancelEvent)
{
	int called = 0;
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	auto ev = std::make_unique<MyTestEvent>([&called] { ++called; });

	auto pev = ev.get();

	dcclite::broker::EventHub::MakeEvent<MyTestEvent>([&called] { ++called;  });

	dcclite::broker::EventHub::PostEvent(std::move(ev));

	dcclite::broker::EventHub::MakeEvent<MyTestEvent>([&called] { ++called; });


	ASSERT_EQ(MyTestEvent::GetObjectCount(), 3);	

	dcclite::broker::EventHub::CancelEvents(pev->GetTarget());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 2);	
	ASSERT_EQ(called, 0);

	dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
	ASSERT_EQ(called, 2);
}