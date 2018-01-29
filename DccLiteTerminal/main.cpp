#include <iostream>

#include "LogUtils.h"

#include "NetMessenger.h"

#include <plog/log.h>

using namespace dcclite;

int main(int, char **)
{
	InitLog("terminal.log");

	LOG_INFO << "Trying to connect...";

	Socket socket{};	

	if (!socket.TryOpen(0, Socket::Type::STREAM))
	{
		LOG_ERROR << "Cannot create socket";

		return -1;
	}

	if (!socket.TryConnect(Address(127, 0, 0, 1, 4190)))
	{
		LOG_ERROR << "Cannot connect to server";

		return -1;
	}

	LOG_INFO << "Connected to server";

	return 0;
}
