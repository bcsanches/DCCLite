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

#include <RName.h>

using namespace dcclite;

TEST(RName, BasicTest)
{
	RName name1{ "abc" };
	RName name2{ "abc" };
	RName nameInvalid{ std::string{"hello"} };
	RName nameInvalid1{ std::string{"hello"} };

	ASSERT_EQ(nameInvalid, nameInvalid1);

	RName nullName;

	ASSERT_FALSE(nullName);
	ASSERT_TRUE(name1);
	ASSERT_TRUE(name2);

	ASSERT_EQ(name1, name2);

	ASSERT_STREQ(name1.GetData().data(), "abc");

	RName name3{ "xyz" };
	ASSERT_NE(name1, name3);
	ASSERT_NE(name1, nullName);
	

	ASSERT_STREQ(name3.GetData().data(), "xyz");

	ASSERT_STREQ(nullName.GetData().data(), "null_name");
}

TEST(RName, LimitTest)
{

}