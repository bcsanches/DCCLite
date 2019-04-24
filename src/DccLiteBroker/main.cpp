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
#include "Log.h"
#include "LogUtils.h"
#include "PathUtils.h"
#include "TerminalCmd.h"

#include <spdlog/logger.h>

static bool fExitRequested = false;

//static TerminalCmd g_ShutdownCmd{ "shutdown" };

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	fExitRequested = true;

	return true;
}

int main(int argc, char **argv)
{		
	dcclite::LogInit("DccLiteBrooker_%N.log");

	dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);
	dcclite::ConsoleTryMakeNice();

	dcclite::PathUtils::SetAppName("Brooker");

	try
	{ 
		Broker broker{ (argc == 1) ? "MyRailroad" : argv[1] };

		dcclite::Clock clock;

		dcclite::Log::Info("Ready, entering main loop");
		
		while (!fExitRequested)
		{
			if (!clock.Tick(std::chrono::milliseconds{ 100 }))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
				continue;
			}			

			broker.Update(clock);
		}			
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
