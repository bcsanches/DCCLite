// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef CONSOLE_NET_H
#define CONSOLE_NET_H

#include "Strings.h"
#include <Arduino.h>

namespace Console
{
	typedef __FlashStringHelper ConsoleFlashStringHelper_t;	

	extern void Printf(const ConsoleFlashStringHelper_t *format, ...);
	
	extern void Init();		

	extern void Update();

	extern bool Custom_ParseCommand(dcclite::StringView command);
};

#endif
