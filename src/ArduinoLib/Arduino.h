#pragma once

#include "ArduinoLib.h"
#include "Serial.h"

#define BCS_ARDUINO_EMULATOR 1

enum PinModes
{
	OUTPUT,
	INPUT,
	INPUT_PULLUP
};


ARDUINO_API extern void pinMode(int pin, PinModes mode);
ARDUINO_API extern int digitalRead(int pin);

ARDUINO_API extern void digitalWrite(int pin, int value);

ARDUINO_API extern unsigned int bitRead(unsigned int flags, int pos);

ARDUINO_API extern unsigned long millis();


