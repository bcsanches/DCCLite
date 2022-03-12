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

#include "Clock.h"
#include "Thinker.h"

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

TEST(Thinker, RunTest)
{					
	g_iCounter = 0;

	dcclite::broker::Thinker ta{ ProcA };
	dcclite::broker::Thinker tb{ ProcB };
	dcclite::broker::Thinker tc{ ProcC };

	dcclite::Clock ck;

	ck.Tick();

	auto tp = ck.Ticks();

	using namespace std::chrono_literals;

	tc.SetNext(tp + 150ms);
	ta.SetNext(tp + 50ms);
	tb.SetNext(tp + 100ms);	

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
	tc.SetNext(tp + 150ms);
	ta.SetNext(tp + 50ms);
	tb.SetNext(tp + 100ms);

	dcclite::broker::Thinker::UpdateThinkers(tp + 300ms);
	ASSERT_EQ(g_iCounter, 3);
}