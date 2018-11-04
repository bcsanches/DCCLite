#include <iostream>
#include <signal.h>
#include <stdexcept>

#include "Brooker.h"

#include "ConsoleUtils.h"
#include "LogUtils.h"
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

	const char *configFileName = (argc == 1) ? "config.json" : argv[1];	

	try
	{ 
		Brooker brooker;

		brooker.LoadConfig(configFileName);
		
		while(!fExitRequested)
			brooker.Update();
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
