#include <iostream>
#include <signal.h>
#include <stdexcept>

#include <plog/Log.h>

#include "Brooker.h"

#include "ConsoleUtils.h"
#include "LogUtils.h"
#include "TerminalCmd.h"

static bool fExitRequested = false;

static TerminalCmd g_ShutdownCmd{ "shutdown" };

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	fExitRequested = true;

	return true;
}

int main(int argc, char **argv)
{
	dcclite::InitLog("DccLiteBrooker_%N.log");	

	dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

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
		LOG_FATAL << "caught " << ex.what();
	}

	return 0;
}
