// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <iostream>
#include <signal.h>
#include <stdexcept>

#include "Broker.h"

#include "Clock.h"
#include "ConsoleUtils.h"
#include "FileWatcher.h"
#include "Log.h"
#include "LogUtils.h"
#include "PathUtils.h"
#include "TerminalCmd.h"

#include <spdlog/logger.h>

constexpr auto BUILD_NUM = "0.3.0";


static bool fExitRequested = false;

//static TerminalCmd g_ShutdownCmd{ "shutdown" };

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	fExitRequested = true;

	return true;
}

int main(int argc, char **argv)
{				
	try
	{ 
		dcclite::PathUtils::InitAppFolders("Broker");

		dcclite::LogInit("DccLiteBroker_%N.log");

#ifndef DEBUG
		dcclite::LogGetDefault()->set_level(spdlog::level::info);
#endif

		dcclite::Log::Info("DCClite {} {}", BUILD_NUM, __DATE__);

		dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

		dcclite::ConsoleTryMakeNice();

		Broker broker{ (argc == 1) ? "MyRailroad" : argv[1] };

		dcclite::Clock clock;

		dcclite::Log::Info("Ready, main loop...");
		
		while (!fExitRequested)
		{
			if (!clock.Tick(std::chrono::milliseconds{ 100 }))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
				continue;
			}			

			FileWatcher::PumpEvents();
			broker.Update(clock);
		}			
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
