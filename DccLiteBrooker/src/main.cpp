#include <iostream>
#include <stdexcept>

#include <boost/log/core.hpp>
#include <boost/log/expressions/predicates/is_debugger_present.hpp>
#include <boost/log/sinks/debug_output_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "Brooker.h"

static void InitLog()
{
	namespace blog = boost::log;
	namespace sinks = boost::log::sinks;
	namespace keywords = boost::log::keywords;

	blog::add_file_log(
		keywords::file_name = "DccLiteBrooker_%N.log",
		keywords::rotation_size = 10 * 1024 * 1024,
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
		keywords::format = "[%TimeStamp%]: %Message%"
	);

	blog::add_console_log();

	auto core = blog::core::get();

	typedef sinks::synchronous_sink< sinks::debug_output_backend > sink_t;

	// Create the sink. The backend requires synchronization in the frontend.
	boost::shared_ptr< sink_t > sink(new sink_t());

	// Set the special filter to the frontend
	// in order to skip the sink when no debugger is available
	sink->set_filter(blog::expressions::is_debugger_present());

	core->add_sink(sink);
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
		BOOST_LOG_TRIVIAL(fatal) << "caught: " << ex.what();	
	}

	return 0;
}
