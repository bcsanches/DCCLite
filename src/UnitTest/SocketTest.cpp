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

#include "Socket.h"

using namespace dcclite;

TEST(Socket, NetworkAddressParse)
{				
	ASSERT_THROW(NetworkAddress::ParseAddress("127"), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("a"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127:"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127."), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("127.a"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0"), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("127.0."), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.z"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0"), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0z"), std::invalid_argument);
	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0."), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0.1"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0.1."), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0.1:"), std::invalid_argument);

	ASSERT_THROW(NetworkAddress::ParseAddress("127.0.0.1:a"), std::invalid_argument);


	auto addr = NetworkAddress::ParseAddress("127.1.2.3:2560");	

	ASSERT_EQ(addr.GetA(), 127);
	ASSERT_EQ(addr.GetB(), 1);
	ASSERT_EQ(addr.GetC(), 2);
	ASSERT_EQ(addr.GetD(), 3);

	ASSERT_EQ(addr.GetPort(), 2560);
}