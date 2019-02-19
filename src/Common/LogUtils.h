#pragma once

#include <memory>

namespace spdlog
{
	class logger;
}

namespace dcclite
{
	typedef std::shared_ptr<spdlog::logger> Logger_t;

	extern void LogInit(const char *fileName);

	extern void LogReplace(Logger_t log);

	extern Logger_t LogGetDefault();
	//inline Logger_t Log() { return LogGetDefault(); }
}

