#include "Arduino.h"

#include <array>
#include <chrono>

#include "ArduinoLib.h"

#include "DynamicLibrary.h"

#include "EEPROMLib.h"

using namespace std;

#define MAX_PINS 64

namespace ArduinoLib
{
	namespace detail
	{
		//
		//
		// Time Facilities
		//
		//
		typedef chrono::high_resolution_clock DefaultClock_t;

		static std::chrono::time_point<DefaultClock_t> g_StartTime;
		static std::chrono::time_point<DefaultClock_t> g_CurrentTime;

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
					//default mode is with pullup resistor
					m_eMode(INPUT_PULLUP),
					m_eVoltage(HIGH)
				{
					//empty
				}

				void setPinMode(PinModes mode)
				{
					m_eMode = mode;

					if (mode != INPUT_PULLUP)
						m_eVoltage = LOW;
				}

				void digitalWrite(VoltageModes mode)
				{
					m_eVoltage = mode;
				}

				int digitalRead()
				{
					return m_eVoltage;
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

	void Setup(std::string moduleName, dcclite::Logger_t log)
	{
		g_ModuleLib.Load(moduleName);

		g_strModuleName = std::move(moduleName);

		g_pfnSetup = static_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("setup"));
		g_pfnLoop = static_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("loop"));		

		g_CurrentTime = g_StartTime = DefaultClock_t::now();

		detail::RomSetupModule(g_strModuleName, log);

		//initialize client
		g_pfnSetup();
	}

	void Finalize()
	{
		detail::RomFinalize();
	}

	void Tick()
	{
		g_CurrentTime = DefaultClock_t::now();

		g_Millis = static_cast<unsigned long>(chrono::duration_cast<chrono::milliseconds>(g_CurrentTime - g_StartTime).count());

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

