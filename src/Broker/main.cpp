// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <chrono>

#include <Clock.h>
#include <ConsoleUtils.h>
#include <Log.h>
#include <LogUtils.h>
#include <PathUtils.h>


#include "sys/Broker.h"
#include "sys/EventHub.h"
#include "sys/Thinker.h"


#include <spdlog/logger.h>

constexpr auto BUILD_NUM = DCCLITE_VERSION;

static bool g_fExitRequested = false;

class NullEventTarget: public dcclite::broker::EventHub::IEventTarget
{
	public:
		//empty
};

class QuitEvent: public dcclite::broker::EventHub::IEvent
{
	public:
		QuitEvent(dcclite::broker::EventHub::IEventTarget &target) :
			IEvent{ target }
		{
			//empty
		}

		void Fire() override
		{
			//nothing todo, this is used just to wake up the main thread
		}
};

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	g_fExitRequested = true;

	static NullEventTarget g_NullTarget;

	//wake up main thread if it is sitting waiting for events...
	dcclite::broker::EventHub::PostEvent<QuitEvent>(std::ref(g_NullTarget));

	dcclite::Log::Info("[Main] CTRL+C detected, exiting...");

	return true;
}

int main(int argc, char **argv)
{			
	using namespace std::chrono_literals;

	try
	{ 
		dcclite::PathUtils::InitAppFolders("Broker");

		dcclite::LogInit("DccLiteBroker.log");

#ifndef DEBUG
		dcclite::LogGetDefault()->set_level(spdlog::level::trace);
#else
		dcclite::LogGetDefault()->set_level(spdlog::level::trace);
#endif

		dcclite::Log::Info("DCClite {} {}", BUILD_NUM, __DATE__);

		dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

		dcclite::ConsoleTryMakeNice();

		dcclite::broker::Broker broker{ (argc == 1) ? "MyRailroad" : argv[1] };
		
		dcclite::Log::Info("Ready, main loop...");

		unsigned frameCount = 0;
		auto startTime = dcclite::Clock::DefaultClock_t::now();
		
		while (!g_fExitRequested)
		{			
			auto now = dcclite::Clock::DefaultClock_t::now();

			++frameCount;			
			if ((now - startTime) >= std::chrono::seconds{ 1 })
			{
				startTime += std::chrono::seconds{ 1 };

				//dcclite::Log::Debug("[{}]", frameCount);
				frameCount = 0;
			}
			
			auto timeout = dcclite::broker::Thinker::UpdateThinkers(now);
			
			dcclite::broker::EventHub::PumpEvents(timeout);						
		}			
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	dcclite::Log::Info("[Main] Bye");

	return 0;
}
