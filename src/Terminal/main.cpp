// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <iostream>

#include <dcclite/dcclite.h>
#include <dcclite/Log.h>
#include <dcclite/NetMessenger.h>

using namespace dcclite;

int main(int, char **)
{
	dcclite::Init("Terminal", "terminal.log");		

	dcclite::Log::Info("Trying to connect...");	

	Socket socket{};	

	if (!socket.Open(0, Socket::Type::STREAM))
	{
		dcclite::Log::Error("Cannot create socket");

		return -1;
	}

	if (!socket.StartConnection(NetworkAddress(127, 0, 0, 1, 4190)))
	{
		dcclite::Log::Error("Cannot connect to server");

		return -1;
	}

	while (socket.GetConnectionProgress() == Socket::Status::WOULD_BLOCK);

	auto info = socket.GetConnectionProgress();

	dcclite::Log::Info("Connection: {}", info == Socket::Status::OK ? "OK" : "FAILED");

	return 0;
}
