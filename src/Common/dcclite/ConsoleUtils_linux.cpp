// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ConsoleUtils.h"

#include <signal.h>

static dcclite::ConsoleEventCallback_t g_Callback = nullptr;

static void SignalHandler(int s) 
{
	switch (s)
	{
		case SIGINT:
		case SIGKILL:
		case SIGQUIT:
		case SIGSTOP:
		case SIGTERM:
		case SIGABRT:		
		case SIGPWR:
			g_Callback(dcclite::ConsoleEvent::CLOSE);
	}	
}

namespace dcclite
{
	bool ConsoleTryMakeNice()
	{
		//nothing to do, it is already nice
		return true;
	}

	void ConsoleInstallEventHandler(ConsoleEventCallback_t callback)
	{
		g_Callback = callback;

		signal(SIGINT, SignalHandler);		
	}
}


