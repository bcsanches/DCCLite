#pragma once

//Due to stupid headers, like windows.h and Arduino.h, we need to split the files to try to avoid name conflict

#include "ArduinoLibDefs.h"

#include <string>

enum VoltageModes
{
	LOW,
	HIGH
};

namespace ArduinoLib
{
	typedef void(*ArduinoProc_t)();

	ARDUINO_API void Setup(std::string moduleName);

	ARDUINO_API void Tick();

	ARDUINO_API void SetSerialInput(const char *data);

	ARDUINO_API void SetPinDigitalVoltage(int pin, VoltageModes voltage);
}


