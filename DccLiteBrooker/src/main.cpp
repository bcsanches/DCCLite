#include <iostream>
#include <stdexcept>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "Brooker.h"

static void InitLog()
{
	namespace blog = boost::log;
	namespace sinks = boost::log::sinks;
	namespace keywords = boost::log::keywords;

	blog::add_console_log();

	blog::add_file_log(
		keywords::file_name = "DccLiteBrooker_%N.log",
		keywords::rotation_size = 10 * 1024 * 1024,
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
		keywords::format = "[%TimeStamp%]: %Message%"
	);
}

int main(int argc, char **argv)
{
	InitLog();

	// Here we go, we can write logs right away
	BOOST_LOG_TRIVIAL(info) << "Hello";

	const char *configFileName = (argc == 1) ? "config.json" : argv[1];	

	try
	{ 
		Brooker brooker;

		brooker.LoadConfig(configFileName);
	}	
	catch (std::exception &ex)
	{
		std::cerr << "caught: " << ex.what();
	}

	return 0;
}
