// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Helpers.h"

#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include "sys/Project.h"
#include "sys/ServiceFactory.h"
#include "tycoon/TycoonService.h"

using namespace dcclite;
using namespace dcclite::broker::tycoon;
using namespace dcclite::broker::sys;

using namespace rapidjson;

std::unique_ptr<TycoonService> LoadTycoon(const char *json, bool deleteExistingState)
{
	Document d;
	d.Parse(json);
	auto obj = d.GetObject();

	Project::SetName("TycoonUnitTest");

	if (deleteExistingState)
	{
		auto stateFileName = Project::GetAppFilePath("tycoon.state.json");
		dcclite::fs::remove(stateFileName);
	}

	TycoonService::RegisterFactory();

	auto ptr = ServiceFactory::TryFindFactory(RName{ "TycoonService" })->Create(
		RName{ "tycoon" },
		*static_cast<Broker *>(nullptr),
		obj
	);

	return std::unique_ptr<TycoonService>{ static_cast<TycoonService *>(ptr.release()) };
}

void CheckException(std::function<void()> lambda, const char *expectedMsg)
{
	try
	{
		lambda();

		FAIL() << "Expected exception!!";
	}
	catch (std::exception &ex)
	{
		ASSERT_STREQ(
			ex.what(),
			expectedMsg
		);
	}
}

void CheckLoadException(const char *json, const char *expectedMessage)
{
	CheckException([json]()
		{
			LoadTycoon(json);

			FAIL() << "Expected exception!!";
		},
		expectedMessage
	);	
}

void Tick(int fastClockTicks)
{
	using namespace dcclite::broker::sys;

	for (int i = 0; i < fastClockTicks; ++i)
	{
		auto first = Thinker::TryGetFirstThinker();
		ASSERT_TRUE(first);

		dcclite::broker::sys::Thinker::UpdateThinkers(first->GetTimePoint());
	}
}
