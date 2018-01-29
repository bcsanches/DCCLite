#include <iostream>

#include "LogUtils.h"

#include <plog/log.h>

int main(int, char **)
{
	dcclite::InitLog("terminal.log");

	LOG_INFO << "hello";

	return 0;
}
