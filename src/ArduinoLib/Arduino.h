#pragma once

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

extern void pinMode(int pin, PinModes mode);
extern int digitalRead(int pin);

extern void digitalWrite(int pin, int value);

extern unsigned int bitRead(unsigned int flags, int pos);

extern unsigned long millis();

//
// Client functions that should be implemented
//
extern void setup();
extern void loop();

namespace ArduinoLib
{
	void setup();

	void tick();

	void setSerialInput(const char *data);

	void setPinDigitalVoltage(int pin, VoltageModes voltage);
}
