// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "LogUtils.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "PathUtils.h"

#ifdef WIN32
	#include <spdlog/sinks/msvc_sink.h>	
#endif

namespace dcclite
{
	static std::shared_ptr<spdlog::logger> g_spLogger;

	void LogFinalize()
	{
		g_spLogger.reset();

		spdlog::drop("dcclite");

		spdlog::shutdown();
	}
	
	void LogInit(const char *fileName)
	{
		//auto console = spdlog::stdout_color_mt("console");

#ifdef WIN32
		auto logPath = dcclite::PathUtils::GetAppFolder().string();
		logPath.append(fileName);
#else
		dcclite::fs::path logPath{ fileName };
#endif
		
		std::vector<spdlog::sink_ptr> sinks;		
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());		
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath, true));		

#ifdef WIN32
		sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif		
		auto combined_logger = std::make_shared<spdlog::logger>("dcclite", begin(sinks), end(sinks));

		//register it if you need to access it globally
		spdlog::register_logger(combined_logger);		
					
		combined_logger->set_level(spdlog::level::trace); // Set specific logger's log level		

		// Customize msg format for all loggers
		spdlog::set_pattern("[%T] [%^-%L-%$] [T %t] %v");		
		
		combined_logger->flush_on(spdlog::level::err);
		
		combined_logger->info("Log started");

#ifdef WIN32
		// can be set globally or per logger(logger->set_error_handler(..))
		spdlog::set_error_handler([](const std::string &msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });
#endif

		g_spLogger = combined_logger;
	}

	void LogReplace(Logger_t log)
	{
		g_spLogger = log;
	}

	extern std::shared_ptr<spdlog::logger> LogGetDefault()
	{		
		return g_spLogger;
	}
}
