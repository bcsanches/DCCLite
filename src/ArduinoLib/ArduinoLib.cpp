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
	}

	static DynamicLibrary g_ModuleLib;
	static std::string g_strModuleName;
	static std::string g_strDeviceName;

	static ArduinoProc_t g_pfnSetup;
	static ArduinoProc_t g_pfnLoop;

	bool Setup(std::string moduleName, dcclite::Logger_t log, const char *deviceName)
	{
		dcclite::Log::Replace(log);

		g_ModuleLib.Load(moduleName);

		g_strModuleName = std::move(moduleName);
		g_strDeviceName = deviceName ? deviceName : "";

		g_pfnSetup = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("setup"));
		g_pfnLoop = reinterpret_cast<ArduinoProc_t>(g_ModuleLib.GetSymbol("loop"));

		detail::BoardInit();

		bool romResult = detail::RomSetupModule(!g_strDeviceName.empty() ? g_strDeviceName : g_strModuleName);

		//initialize client
		g_pfnSetup();

		return romResult;
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

	void Tick()
	{
		detail::BoardTick();

		//run client loop
		g_pfnLoop();

		detail::RomAfterLoop();
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



