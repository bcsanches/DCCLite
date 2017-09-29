#include "LogUtils.h"

#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4819)

#	include <boost/log/core.hpp>
#	include <boost/log/expressions.hpp>
#	include <boost/log/expressions/predicates/is_debugger_present.hpp>
#	include <boost/log/expressions/formatters/date_time.hpp>
#	include <boost/log/sinks/debug_output_backend.hpp>
#	include <boost/log/support/date_time.hpp>
#	include <boost/log/trivial.hpp>
#	include <boost/log/utility/setup/common_attributes.hpp>
#	include <boost/log/utility/setup/console.hpp>
#	include <boost/log/utility/setup/file.hpp>
#pragma warning(pop)

namespace dcclite
{
	//Thanks to https://gist.github.com/xiongjia/e23b9572d3fc3d677e3d
	void InitLog(const char *fileName)
	{
		namespace blog = boost::log;
		namespace sinks = boost::log::sinks;
		namespace keywords = boost::log::keywords;
		namespace exprs = blog::expressions;

		blog::add_common_attributes();

		/* log formatter:
		* [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
		*/
		auto fmtTimeStamp = boost::log::expressions::
			format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S");
		auto fmtThreadId = boost::log::expressions::
			attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");
		auto fmtSeverity = boost::log::expressions::
			attr<boost::log::trivial::severity_level>("Severity");

		/*
		auto fmtScope = boost::log::expressions::format_named_scope("Scope",
		boost::log::keywords::format = "%n(%f:%l)",
		boost::log::keywords::iteration = boost::log::expressions::reverse,
		boost::log::keywords::depth = 2);
		*/

		boost::log::formatter logFmt =
			boost::log::expressions::format("[%1%] (%2%) [%3%] %4%")
			% fmtTimeStamp % fmtThreadId % fmtSeverity
			% boost::log::expressions::smessage;

		auto fileLogSink = blog::add_file_log(
			keywords::file_name = fileName,
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)
		);

		fileLogSink->set_formatter(logFmt);
		fileLogSink->locked_backend()->auto_flush(true);

		auto consoleSink = blog::add_console_log();
		consoleSink->set_formatter(logFmt);

		auto core = blog::core::get();

		typedef sinks::synchronous_sink< sinks::debug_output_backend > sink_t;

		// Create the sink. The backend requires synchronization in the frontend.
		boost::shared_ptr< sink_t > sink(new sink_t());

		// Set the special filter to the frontend
		// in order to skip the sink when no debugger is available
		sink->set_filter(blog::expressions::is_debugger_present());

		core->add_sink(sink);
	}
}