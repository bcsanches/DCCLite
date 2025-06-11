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
#include <thread>

#include <dcclite/Clock.h>

#include "ArduinoLib.h"

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

		static unsigned long g_Millis = 0;

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
					m_eVoltage = mode;
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

		void BoardInit()
		{
			g_Clock = dcclite::Clock();
			g_Millis = 0;
		}

		void BoardTick()
		{
			g_Clock.Tick();

			g_Millis = static_cast<unsigned long>(g_Clock.Total().count());
		}

		void BoardFixedTick(unsigned long ms)
		{
			g_Millis += ms;
		}

		void BoardFinalize()
		{
			//empty
		}
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
	return ArduinoLib::detail::digitalRead(pin);
}

unsigned int bitRead(unsigned int flags, int pos)
{
	return (flags >> pos) & 1;
}

void delay(unsigned long ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds{ ms });
}

//
//
// Emulator Entry Point
//
//

void ArduinoLib::SetPinDigitalVoltage(int pin, VoltageModes voltage)
{
	detail::g_Pins.at(pin).setDigitalVoltage(voltage);
}

