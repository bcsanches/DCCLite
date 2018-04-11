#include "LogUtils.h"

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

namespace dcclite
{
	//https://github.com/SergiusTheBest/plog#logger
	void InitLog(const char *fileName)
	{
		static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;

		plog::init(plog::verbose, fileName);
		plog::Logger<0>::getInstance()->addAppender(&consoleAppender);
	}
}
