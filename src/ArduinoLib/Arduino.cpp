// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Arduino.h"

#include <array>
#include <chrono>

#include "ArduinoLib.h"
#include "Clock.h"

#include "DynamicLibrary.h"

#include "EEPROMLib.h"

using namespace std;

#define MAX_PINS 70

namespace ArduinoLib
{
	namespace detail
	{
		//
		//
		// Time Facilities
		//
		//
		dcclite::Clock g_Clock;

		static unsigned long g_Millis;

		//
		//
		// Pins Control
		//
		//

		class ArduinoPin
		{
			public:
				ArduinoPin() :
					//default mode is INPUT LOW
					m_eMode(INPUT),
					m_eVoltage(LOW)
				{
					//empty
				}

				void setPinMode(PinModes mode)
				{
					m_eMode = mode;

					if (mode == INPUT_PULLUP)
						m_eVoltage = HIGH;
					else if (mode == INPUT)
						m_eVoltage = LOW;
				}

				void digitalWrite(VoltageModes voltage)
				{
					if (m_eMode != OUTPUT)
					{
						//writing HIGH to input pin turn on PULLUP
						setPinMode(voltage == HIGH ? INPUT_PULLUP : INPUT);
					}
					else
					{
						m_eVoltage = voltage;
					}
				}

				int digitalRead()
				{
					return m_eMode == INPUT_PULLUP ? (m_eVoltage == HIGH ? LOW : HIGH) : m_eVoltage;
				}

				void setDigitalVoltage(VoltageModes mode)
				{
					this->digitalWrite(mode);
				}

			private:
				PinModes m_eMode;
				VoltageModes m_eVoltage;
		};

		static array<ArduinoPin, MAX_PINS> g_Pins;

		//
		//
		// Arduino Lib helpers
		//
		//

		static void pinMode(int pin, PinModes mode)
		{
			g_Pins.at(pin).setPinMode(mode);
		}

		static void digitalWrite(int pin, VoltageModes mode)
		{
			g_Pins.at(pin).digitalWrite(mode);
		}

		static int digitalRead(int pin)
		{
			return g_Pins.at(pin).digitalRead();
		}

		//
		//
		//

		static ArduinoProc_t g_pfnSetup;
		static ArduinoProc_t g_pfnLoop;
	}
	
	using namespace detail;

	//
	//
	// Emulator Entry points
	//
	//

	DynamicLibrary g_ModuleLib;
	std::string g_strModuleName;

	void Setup(std::string moduleName, dcclite::Logger_t log, const char *deviceName)
	{
		dcclite::LogReplace(log);

#error todo

		//
		//Check if device name rom exists, if not, load the module, init session, save it... unload module and continue

		g_ModuleLib.Load(moduleName);

		g_strModuleName = std::move(moduleName);

		g_pfnSetup = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("setup"));
		g_pfnLoop = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("loop"));

		g_Clock = dcclite::Clock();

		detail::RomSetupModule(deviceName ? deviceName : g_strModuleName);

		//initialize client
		g_pfnSetup();
	}

	void Finalize()
	{
		detail::RomFinalize();
	}

	void Tick()
	{
		g_Clock.Tick();

		g_Millis = static_cast<unsigned long>(g_Clock.Total().count());

		//run client loop
		g_pfnLoop();

		detail::RomAfterLoop();
	}

	void SetSerialInput(const char *data) 
	{
		Serial.internalSetData(data);
	}

	void SetPinDigitalVoltage(int pin, VoltageModes voltage)
	{
		g_Pins.at(pin).setDigitalVoltage(voltage);
	}
}

//
//
// Arduino Library implementation
//
//

unsigned long millis()
{
	return ArduinoLib::detail::g_Millis;
}

void pinMode(int pin, PinModes mode)
{
	ArduinoLib::detail::pinMode(pin, mode);
}

void digitalWrite(int pin, int value)
{
	ArduinoLib::detail::digitalWrite(pin, value ? HIGH : LOW);
}

int digitalRead(int pin)
{
	return ArduinoLib::digitalRead(pin);
}

unsigned int bitRead(unsigned int flags, int pos)
{
	return (flags >> pos) & 1;
}

