// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

namespace dcclite
{
	enum class ConsoleEvent
	{
		CTRL_C,
		CTRL_BREAK,
		CLOSE,
		LOGOFF,
		SHUTDOWN
	};

	typedef bool (*ConsoleEventCallback_t)(ConsoleEvent );

	extern void ConsoleInstallEventHandler(ConsoleEventCallback_t callback);

	extern bool ConsoleTryMakeNice();
}

