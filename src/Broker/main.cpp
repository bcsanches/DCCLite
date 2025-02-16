// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <atomic>
#include <chrono>

#include <dcclite/Clock.h>
#include <dcclite/ConsoleUtils.h>
#include <dcclite/Log.h>
#include <dcclite/LogUtils.h>
#include <dcclite/PathUtils.h>

#include "sys/Broker.h"
#include "sys/EventHub.h"
#include "sys/Thinker.h"

#include <spdlog/logger.h>

constexpr auto BUILD_NUM = DCCLITE_VERSION;

static std::atomic_flag g_fExitRequested;

class QuitEvent: public dcclite::broker::EventHub::IEvent
{
	private:
		class NullEventTarget: public dcclite::broker::EventHub::IEventTarget
		{
			//empty
		};

		static NullEventTarget g_clTarget;

	public:
		QuitEvent() :
			IEvent{ std::ref(g_clTarget)}
		{
			//empty
		}

		void Fire() override
		{
			//nothing todo, this is used just to wake up the main thread
		}
};

QuitEvent::NullEventTarget QuitEvent::g_clTarget;

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	g_fExitRequested.test_and_set(std::memory_order_relaxed);

	//wake up main thread if it is sitting waiting for events...
	dcclite::broker::EventHub::PostEvent<QuitEvent>();

	dcclite::Log::Info("[Main] CTRL+C detected, exiting...");

	//give some time for main thread to finish up... so it can do the cleanup
	using namespace std::chrono_literals;

	//if we return befor the main thread, it may not finish cleaning up...
	while(g_fExitRequested.test())
		std::this_thread::sleep_for(1000ms);

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
		dcclite::Log::Info("[DccLite] Working dir: {}", dcclite::fs::current_path().string());

		dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

		dcclite::ConsoleTryMakeNice();		

		dcclite::broker::Broker broker{ (argc == 1) ? "MyRailroad" : argv[1] };		
		
		dcclite::Log::Info("Ready, main loop...");		
		
		while (!g_fExitRequested.test(std::memory_order_relaxed))
		{			
			auto now = dcclite::Clock::DefaultClock_t::now();
						
			auto timeout = dcclite::broker::Thinker::UpdateThinkers(now);
			
			dcclite::broker::EventHub::PumpEvents(timeout);
		}
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}
	
	dcclite::Log::Info("[Main] Bye");

	//notify console handler that we are done
	g_fExitRequested.clear(std::memory_order_relaxed);	

	return 0;
}
