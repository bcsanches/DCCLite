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


