#pragma once

#include "ArduinoLibDefs.h"
#include "Serial.h"

#define BCS_ARDUINO_EMULATOR 1

enum PinModes
{
	OUTPUT,
	INPUT,
	INPUT_PULLUP
};

enum VoltageModes
{
	LOW,
	HIGH
};

ARDUINO_API extern void pinMode(int pin, PinModes mode);
ARDUINO_API extern int digitalRead(int pin);

ARDUINO_API extern void digitalWrite(int pin, int value);

ARDUINO_API extern unsigned int bitRead(unsigned int flags, int pos);

ARDUINO_API extern unsigned long millis();

typedef void(*ArduinoProc_t)();

namespace ArduinoLib
{
	ARDUINO_API void setup(ArduinoProc_t pfnSetup, ArduinoProc_t pfnLoop);

	ARDUINO_API void tick();

	ARDUINO_API void setSerialInput(const char *data);

	ARDUINO_API void setPinDigitalVoltage(int pin, VoltageModes voltage);
}
