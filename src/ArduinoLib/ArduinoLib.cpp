// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ArduinoLib.h"

#include <dcclite/Log.h>

#include "avr/wdt.h"
#include "DynamicLibrary.h"
#include "EEPROMLib.h"
#include "Ethercard.h"
#include "Serial.h"

namespace ArduinoLib
{
	namespace detail
	{
		void BoardInit();
		void BoardFinalize();
		void BoardTick();
		void BoardFixedTick(unsigned long msstep);
	}

	static DynamicLibrary g_ModuleLib;
	static std::string g_strModuleName;
	static std::string g_strDeviceName;

	static ArduinoProc_t g_pfnSetup;
	static ArduinoProc_t g_pfnLoop;


	static bool ReSetup()
	{
		if (!g_strModuleName.empty())
		{
			g_ModuleLib.Load(g_strModuleName);

			g_pfnSetup = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("setup"));
			g_pfnLoop = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("loop"));
		}

		detail::BoardInit();

		bool romResult = detail::RomSetupModule(!g_strDeviceName.empty() ? g_strDeviceName : g_strModuleName);

		wdt_disable();

		//initialize client
		g_pfnSetup();

		return romResult;
	}

	bool Setup(void (*setupProc)(), void (*loopProc)(), const char *deviceName)
	{
		if (!setupProc)
			throw std::invalid_argument("setup proc cannot be null");

		if(!loopProc)
			throw std::invalid_argument("loop proc cannot be null");

		g_strModuleName.clear();

		g_strDeviceName = deviceName;

		g_pfnSetup = setupProc;
		g_pfnLoop = loopProc;

		return ReSetup();		
	}

	bool Setup(std::string moduleName, dcclite::Logger_t log, const char *deviceName)
	{
		dcclite::Log::Replace(log);		

		g_strModuleName = std::move(moduleName);
		g_strDeviceName = deviceName ? deviceName : "";		

		return ReSetup();
	}

	void Finalize()
	{
		detail::RomFinalize();

		//hack to reset ehtercard lib
		ether.udpServerPauseListenOnPort(0);

		g_pfnLoop = nullptr;
		g_pfnSetup = nullptr;

		g_ModuleLib.Unload();
	}

	static void CommonTick()
	{
		if (detail::WdtExpired())
		{
			//Watch dog fired, restart board...
			Finalize();

			ReSetup();
		}

		//run client loop
		g_pfnLoop();

		detail::RomAfterLoop();
	}

	void FixedTick(unsigned long ms)
	{		
		detail::BoardFixedTick(ms);

		CommonTick();
	}

	void Tick()
	{
		detail::BoardTick();

		CommonTick();
	}

	void SetSerialInput(const char *data)
	{
		Serial.internalSetData(data);
	}

	void SetSerialInput(std::string data)
	{
		Serial.internalSetData(data);
	}	
}



