#include <iostream>

#include <spdlog/logger.h>

#include "LogUtils.h"

#include "NetMessenger.h"

using namespace dcclite;

int main(int, char **)
{
	LogInit("terminal.log");

	auto log = LogGetDefault();

	log->info("Trying to connect...");	

	Socket socket{};	

	if (!socket.Open(0, Socket::Type::STREAM))
	{
		log->error("Cannot create socket");

		return -1;
	}

	if (!socket.StartConnection(Address(127, 0, 0, 1, 4190)))
	{
		log->error("Cannot connect to server");

		return -1;
	}

	while (socket.GetConnectionProgress() == Socket::Status::WOULD_BLOCK);

	auto info = socket.GetConnectionProgress();

	log->info("Connection: {}", info == Socket::Status::OK ? "OK" : "FAILED");

	return 0;
}
