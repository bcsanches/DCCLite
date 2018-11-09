#pragma once

#include <string>

namespace ArduinoLib::detail
{
	void RomSetupModule(std::string_view moduleName);

	void RomAfterLoop();

	void RomFinalize();
}


