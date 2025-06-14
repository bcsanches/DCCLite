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

//Due to stupid headers, like windows.h and Arduino.h, we need to split the files to try to avoid name conflict

#include "ArduinoLibDefs.h"
#include "ArduinoDefs.h"

#include <string>
#include <memory>

namespace spdlog
{
	class logger;
}

namespace dcclite
{
	typedef std::shared_ptr<spdlog::logger> Logger_t;
}

namespace ArduinoLib
{
	typedef void(*ArduinoProc_t)();

	ARDUINO_API bool Setup(std::string moduleName, dcclite::Logger_t log, const char *deviceName);
	ARDUINO_API bool Setup(void (*setupProc)(), void (*loopProc)(), const char *deviceName);

	ARDUINO_API void Finalize();

	ARDUINO_API void Tick();

	ARDUINO_API void FixedTick(unsigned long ms);

	ARDUINO_API void SetSerialInput(const char *data);

	ARDUINO_API void SetPinDigitalVoltage(int pin, VoltageModes voltage);
}


