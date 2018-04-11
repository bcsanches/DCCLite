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

	if (!socket.Open(0, Socket::Type::STREAM))
	{
		LOG_ERROR << "Cannot create socket";

		return -1;
	}

	if (!socket.StartConnection(Address(127, 0, 0, 1, 4190)))
	{
		LOG_ERROR << "Cannot connect to server";

		return -1;
	}

	while (socket.GetConnectionProgress() == Socket::Status::WOULD_BLOCK);

	auto info = socket.GetConnectionProgress();

	LOG_INFO << "Connection: " << (info == Socket::Status::OK ? "OK" : "FAILED");	

	return 0;
}
