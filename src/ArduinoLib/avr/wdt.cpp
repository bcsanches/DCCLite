// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "wdt.h"

#include <exception>

#include "../Arduino.h"

static bool				m_fWdtEnabled = false;
static unsigned long	m_lThinkTime = 0;

void wdt_enable(const uint8_t value)
{
	if (value != WDTO_120MS)
		throw std::exception("Unknow value for wdt_enable");
	
	m_lThinkTime = millis() + 120;
	m_fWdtEnabled = true;
}

void wdt_disable()
{	
	m_fWdtEnabled = false;
}

namespace ArduinoLib::detail
{
	bool WdtExpired()
	{
		return m_fWdtEnabled && (millis() > m_lThinkTime);
	}
}
