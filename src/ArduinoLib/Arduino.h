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
	ARDUINO_API void Setup(std::string moduleName);

	ARDUINO_API void Tick();

	ARDUINO_API void SetSerialInput(const char *data);

	ARDUINO_API void SetPinDigitalVoltage(int pin, VoltageModes voltage);
}
