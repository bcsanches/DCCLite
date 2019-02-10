#include <iostream>
#include <signal.h>
#include <stdexcept>

#include "Brooker.h"

#include "Clock.h"
#include "ConsoleUtils.h"
#include "LogUtils.h"
#include "PathUtils.h"
#include "TerminalCmd.h"

#include <spdlog/logger.h>

static bool fExitRequested = false;

static TerminalCmd g_ShutdownCmd{ "shutdown" };

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
		Brooker brooker{ (argc == 1) ? "MyRailroad" : argv[1] };

		dcclite::Clock clock;
		
		while (!fExitRequested)
		{
			if (!clock.Tick(std::chrono::milliseconds{ 100 }))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
				continue;
			}			

			brooker.Update(clock);
		}			
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
