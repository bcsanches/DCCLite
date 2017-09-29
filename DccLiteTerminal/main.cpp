#include <iostream>

#include "LogUtils.h"

#include <boost\log\trivial.hpp>

int main(int, char **)
{
	dcclite::InitLog("terminal.log");

	BOOST_LOG_TRIVIAL(info) << "hello";

	return 0;
}
