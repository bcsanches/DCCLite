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

#include <dcclite/Object.h>

using namespace dcclite;

	
TEST(ObjectPath, Empty)
{
	ObjectPath path;
				
	ASSERT_TRUE(path.begin() == path.end());
}

TEST(ObjectPath, SingleName)
{
	ObjectPath path("test");

	auto it = path.begin();
	ASSERT_EQ(it.ToString(), std::string("test"));

	++it;
	ASSERT_TRUE(it == path.end());
}

TEST(ObjectPath, Root)
{
	ObjectPath root("/");

	auto it = root.begin();
	ASSERT_EQ(it.ToString(), std::string(1, '/'));

	++it;
	ASSERT_TRUE(it == root.end());
}

TEST(ObjectPath, RootWithSubFolder)
{
	ObjectPath root("/abc/def");

	auto it = root.begin();
	ASSERT_EQ(it.ToString(), std::string(1, '/'));

	++it;
	ASSERT_EQ(it.ToString(), std::string("abc"));

	++it;
	ASSERT_EQ(it.ToString(), std::string("def"));

	++it;

	ASSERT_TRUE(it == root.end());

}

TEST(ObjectPath, RelativeWithSubFolder)
{
	ObjectPath root("abc/def");

	auto it = root.begin();
	ASSERT_EQ(it.ToString(), std::string("abc"));

	++it;
	ASSERT_EQ(it.ToString(), std::string("def"));

	++it;

	ASSERT_TRUE(it == root.end());
}
