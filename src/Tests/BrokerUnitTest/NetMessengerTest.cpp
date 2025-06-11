// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include <dcclite/Socket.h>
#include <dcclite/NetMessenger.h>

using namespace dcclite;
using namespace std::chrono_literals;

TEST(NetMessenger, NetMessengerBasic)
{				
	Socket serverListener{ };
	Socket server;

	ASSERT_TRUE(serverListener.Open(8787, Socket::Type::STREAM));

	ASSERT_TRUE(serverListener.Listen());

	Socket client{};

	ASSERT_TRUE(client.StartConnection(0, Socket::Type::STREAM, NetworkAddress(127, 0, 0, 1, 8787)));
	
	for(;;)
	{
		if (!server.IsOpen())
		{
			auto [status, conSocket, address] = serverListener.TryAccept();
			if (status == Socket::Status::OK)
				server = std::move(conSocket);
		}

		auto status = client.GetConnectionProgress();

		ASSERT_NE(status, Socket::Status::DISCONNECTED);

		if (status == Socket::Status::WOULD_BLOCK)
		{		
			std::this_thread::sleep_for(1ms);
			continue;
		}

		//connected
		break;
	}

	//construct it using smallers separators first
	NetMessenger messenger{ std::move(client), "\r\n"};
	
	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::WOULD_BLOCK);
	}
	

	{
		auto [status, sz] = server.Send("abc", 3);
		ASSERT_EQ(status, Socket::Status::OK);
	}	

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::WOULD_BLOCK);
	}

	{
		auto [status, sz] = server.Send("\r\n", 2);
		ASSERT_EQ(status, Socket::Status::OK);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::OK);

		ASSERT_TRUE(msg.compare("abc") == 0);
	}

	//Check if it will download the first part of the message
	{
		auto [status, sz] = server.Send("abc", 3);
		ASSERT_EQ(status, Socket::Status::OK);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::WOULD_BLOCK);
	}

	//send lots of messages
	{
		auto msg = "abc\rxyz\nppp\r\n";
		auto [status, sz] = server.Send(msg, strlen(msg));
		ASSERT_EQ(status, Socket::Status::OK);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::OK);

		//it cannot mix separators on messages, must be consistent
		//fucking behavior due to JMRI not being consistent on each platform
		ASSERT_TRUE(msg.compare("abcabc\rxyz\nppp") == 0);
	}

	//is it empty?
	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::WOULD_BLOCK);
	}

	//send lots of messages
	{
		auto msg = "abc\r\nxyz\r\nppp\r\n";
		auto [status, sz] = server.Send(msg, strlen(msg));
		ASSERT_EQ(status, Socket::Status::OK);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::OK);
		ASSERT_TRUE(msg.compare("abc") == 0);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::OK);
		ASSERT_TRUE(msg.compare("xyz") == 0);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::OK);
		ASSERT_TRUE(msg.compare("ppp") == 0);
	}

	{
		auto [status, msg] = messenger.Poll();

		ASSERT_EQ(status, Socket::Status::WOULD_BLOCK);
	}
}