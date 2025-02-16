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

#include <dcclite/Clock.h>

#include "sys/Thinker.h"

using namespace dcclite;

static int g_iCounter;

static void ProcA(const dcclite::Clock::TimePoint_t tp)
{
	ASSERT_EQ(g_iCounter, 0);

	g_iCounter = 1;
}

static void ProcB(const dcclite::Clock::TimePoint_t tp)
{
	ASSERT_EQ(g_iCounter, 1);

	g_iCounter = 2;
}

static void ProcC(const dcclite::Clock::TimePoint_t tp)
{
	ASSERT_EQ(g_iCounter, 2);

	g_iCounter = 3;
}

static bool g_fAACalled = false;

static void ProcAA(const dcclite::Clock::TimePoint_t tp)
{	
	g_fAACalled = true;
}

TEST(Thinker, RunTest)
{
	g_iCounter = 0;

	dcclite::broker::Thinker ta{ "ta", ProcA};
	dcclite::broker::Thinker tb{ "tb", ProcB };
	dcclite::broker::Thinker tc{ "tc", ProcC };

	dcclite::Clock ck;

	ck.Tick();

	auto tp = ck.Ticks();

	using namespace std::chrono_literals;

	tc.Schedule(tp + 150ms);
	ta.Schedule(tp + 50ms);
	tb.Schedule(tp + 100ms);

	dcclite::broker::Thinker::UpdateThinkers(tp + 10ms);
	ASSERT_EQ(g_iCounter, 0);

	dcclite::broker::Thinker::UpdateThinkers(tp + 50ms);
	ASSERT_EQ(g_iCounter, 1);

	dcclite::broker::Thinker::UpdateThinkers(tp + 110ms);
	ASSERT_EQ(g_iCounter, 2);

	dcclite::broker::Thinker::UpdateThinkers(tp + 150ms);
	ASSERT_EQ(g_iCounter, 3);

	dcclite::broker::Thinker::UpdateThinkers(tp + 300ms);
	ASSERT_EQ(g_iCounter, 3);

	g_iCounter = 0;
	tc.Schedule(tp + 150ms);
	ta.Schedule(tp + 50ms);
	tb.Schedule(tp + 100ms);

	dcclite::broker::Thinker::UpdateThinkers(tp + 300ms);
	ASSERT_EQ(g_iCounter, 3);

	//
	//Check if multiple run in a row...
	g_iCounter = 0;
	ta.Schedule(tp + 50ms);
	tc.Schedule(tp + 150ms);
	tb.Schedule(tp + 100ms);

	dcclite::broker::Thinker::UpdateThinkers(tp + 100ms);
	ASSERT_EQ(g_iCounter, 2);	
}

TEST(Thinker, SameTime)
{
	g_iCounter = 0;

	dcclite::broker::Thinker ta{ "ta", ProcA };
	dcclite::broker::Thinker tb{ "tb", ProcB };
	dcclite::broker::Thinker tc{ "tc", ProcC };

	dcclite::broker::Thinker taa{ "taa", ProcAA };

	dcclite::Clock ck;

	ck.Tick();

	auto tp = ck.Ticks();

	using namespace std::chrono_literals;

	//	
	ta.Schedule(tp + 10ms);
	tb.Schedule(tp + 100ms);	
	tc.Schedule(tp + 200ms);

	taa.Schedule(tp + 10ms);

	dcclite::broker::Thinker::UpdateThinkers(tp + 50ms);

	ASSERT_EQ(g_iCounter, 1);
	ASSERT_TRUE(g_fAACalled);
}

TEST(Thinker, CancelTest)
{
	g_iCounter = 0;

	dcclite::broker::Thinker ta{ "ta", ProcA };
	dcclite::broker::Thinker tb{ "tb", ProcB };
	dcclite::broker::Thinker tc{ "tc", ProcC };

	dcclite::Clock ck;

	ck.Tick();

	auto tp = ck.Ticks();

	using namespace std::chrono_literals;

	//
	//Check cancel functionality	
	ta.Schedule(tp + 10ms);
	tb.Schedule(tp + 100ms);	
	tb.Cancel();

	dcclite::broker::Thinker::UpdateThinkers(tp + 300ms);
	ASSERT_EQ(g_iCounter, 1);

	g_iCounter = 0;
	ta.Schedule(tp + 10ms);
	tb.Schedule(tp + 100ms);
	tc.Schedule(tp + 200ms);

	tc.Cancel();

	dcclite::broker::Thinker::UpdateThinkers(tp + 300ms);
	ASSERT_EQ(g_iCounter, 2);

	g_iCounter = 0;
	ta.Schedule(tp + 10ms);
	tb.Schedule(tp + 100ms);
	tc.Schedule(tp + 200ms);

	ta.Cancel();

	dcclite::broker::Thinker::UpdateThinkers(tp + 50ms);
	ASSERT_EQ(g_iCounter, 0);
}