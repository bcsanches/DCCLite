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
#include "Log.h"

class EventTargetMockup : public dcclite::broker::EventHub::IEventTarget
{
	public:
		explicit EventTargetMockup(std::string_view name):
			m_svDebugName{name}
		{
			//empty
		}

	private:
		std::string_view m_svDebugName;
};

class MyTestEvent : public dcclite::broker::EventHub::IEvent
{
	public:
		MyTestEvent(dcclite::broker::EventHub::IEventTarget &target, std::function<void()> lambda) :
			IEvent{ target },
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
		std::function<void()> m_pfnLambda;

		static int g_iObjectCount;
};

int MyTestEvent::g_iObjectCount = 0;

TEST(EventHub, Basic)
{					
	bool called = false;

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	EventTargetMockup t1{"t1"};
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { called = true; });

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

	EventTargetMockup t1{"t1"};
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called;  });

	EventTargetMockup t2{"t2"};
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t2), [&called] { ++called;  });

	EventTargetMockup t3{"t3"};
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t3), [&called] { ++called; });


	ASSERT_EQ(MyTestEvent::GetObjectCount(), 3);	

	dcclite::broker::EventHub::CancelEvents(t2);

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 2);	
	ASSERT_EQ(called, 0);

	dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
	ASSERT_EQ(called, 2);
}

TEST(EventHub, TestInternalList)
{
	int called = 0;
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	EventTargetMockup t1{ "t1" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called;  });

	EventTargetMockup t2{ "t2" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t2), [&called] { ++called;  });

	EventTargetMockup t3{ "t3" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t3), [&called] { ++called; });


	ASSERT_EQ(MyTestEvent::GetObjectCount(), 3);

	dcclite::broker::EventHub::CancelEvents(t2);

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 2);
	ASSERT_EQ(called, 0);
	
	dcclite::broker::EventHub::CancelEvents(t1);
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 1);
	
	dcclite::broker::EventHub::CancelEvents(t3);
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
	ASSERT_EQ(called, 0);
}

TEST(EventHub, CancelMultipleTargets)
{
	int called = 0;
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	EventTargetMockup t1{ "t1" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called;  });

	EventTargetMockup t2{ "t2" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t2), [&called] { ++called;  });

	EventTargetMockup t3{ "t3" };
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t3), [&called] { ++called; });
	
	dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called; });


	ASSERT_EQ(MyTestEvent::GetObjectCount(), 4);

	dcclite::broker::EventHub::CancelEvents(t1);

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 2);
	ASSERT_EQ(called, 0);

	dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
	ASSERT_EQ(called, 2);
}

TEST(EventHub, BadAlloc)
{
	dcclite::LogInit("DccliteText.BadAlloc.log");

	int called = 0;
	ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);

	EventTargetMockup t1{ "t1" };

	try
	{
		for (int i = 0;i < 200; ++i)
			dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called;  });

		//fail
		ASSERT_TRUE(false);
	}
	catch (std::bad_alloc &)
	{
		//test passed
		dcclite::broker::EventHub::CancelEvents(t1);

		ASSERT_EQ(MyTestEvent::GetObjectCount(), 0);
		dcclite::broker::EventHub::PumpEvents(dcclite::Clock::DefaultClock_t::now());

		//memory is free
		dcclite::broker::EventHub::PostEvent<MyTestEvent>(std::ref(t1), [&called] { ++called;  });

		dcclite::LogFinalize();

		return;
	}

	ASSERT_TRUE(false);
}