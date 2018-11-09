#pragma once

#include "LogUtils.h"

#include <spdlog/logger.h>

namespace dcclite
{
	namespace Log
	{
		template<typename... Args>
		inline void Trace(const char *fmt, const Args &... args)
		{
			LogGetDefault()->trace(fmt, args...);
		}

		template<typename... Args>
		inline void Debug(const char *fmt, const Args &... args)
		{
			LogGetDefault()->debug(fmt, args...);
		}

		template<typename... Args>
		inline void Info(const char *fmt, const Args &... args)
		{
			LogGetDefault()->info(fmt, args...);
		}

		template<typename... Args>
		inline void Warn(const char *fmt, const Args &... args)
		{
			LogGetDefault()->warn(fmt, args...);
		}

		template<typename... Args>
		inline void Error(const char *fmt, const Args &... args)
		{
			LogGetDefault()->error(fmt, args...);
		}

		template<typename... Args>
		inline void Critical(const char *fmt, const Args &... args)
		{
			LogGetDefault()->critical(fmt, args...);
		}
	}	
}