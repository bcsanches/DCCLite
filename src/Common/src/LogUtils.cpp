#include "LogUtils.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

namespace dcclite
{
	static std::shared_ptr<spdlog::logger> g_spLogger;
	
	void LogInit(const char *fileName)
	{
		//auto console = spdlog::stdout_color_mt("console");

		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true));
		sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
		auto combined_logger = std::make_shared<spdlog::logger>("dcclite", begin(sinks), end(sinks));
		//register it if you need to access it globally
		spdlog::register_logger(combined_logger);		
		
		//console->info("Welcome to spdlog!");
		//console->error("Some error message with arg: {}", 1);		

		// Formatting examples
		//console->warn("Easy padding in numbers like {:08d}", 12);
		//console->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
		//console->info("Support for floats {:03.2f}", 1.23456);
		//console->info("Positional args are {1} {0}..", "too", "supported");
		//console->info("{:<30}", "left aligned");

		//spdlog::get("dcclite")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");

		// Runtime log levels
		//spdlog::set_level(spdlog::level::info); // Set global log level to info
		//console->debug("This message should not be displayed!");
		combined_logger->set_level(spdlog::level::trace); // Set specific logger's log level
		//console->debug("This message should be displayed..");

		// Customize msg format for all loggers
		spdlog::set_pattern("[%T] [%^-%L-%$] [T %t] %v");
		//console->info("This an info message with custom format");		

		combined_logger->flush_on(spdlog::level::err);

		combined_logger->info("Log started");

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