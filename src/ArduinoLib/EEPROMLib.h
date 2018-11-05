#pragma once

#include <string>

#include <LogUtils.h>


namespace ArduinoLib::detail
{
	void RomSetupModule(std::string_view moduleName, dcclite::Logger_t log);

	void RomAfterLoop();

	void RomFinalize();
}


