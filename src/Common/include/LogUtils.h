#pragma once

#include <memory>

namespace spdlog
{
	class logger;
}

namespace dcclite
{
	extern void LogInit(const char *fileName);

	extern std::shared_ptr<spdlog::logger> LogGetDefault();
	inline std::shared_ptr<spdlog::logger> Log() { return LogGetDefault(); }
}

