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

#include "ArduinoDefs.h"
#include "Serial.h"

enum PinModes
{
	OUTPUT,
	INPUT,
	INPUT_PULLUP
};

#define BCS_ARDUINO_EMULATOR 1

ARDUINO_API extern void pinMode(int pin, PinModes mode);
ARDUINO_API extern int digitalRead(int pin);

ARDUINO_API extern void digitalWrite(int pin, int value);

ARDUINO_API extern unsigned int bitRead(unsigned int flags, int pos);

ARDUINO_API extern unsigned long millis();


