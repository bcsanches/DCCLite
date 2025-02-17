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

#include <dcclite_shared/StringView.h>

using namespace dcclite;

TEST(dcclite_shared, StringViewTest)
{
	StringView sv("Hello");

	ASSERT_EQ(sv.GetSize(), 5);
	ASSERT_EQ(sv.Compare("Hello"), 0);

	char str2[] = "Hello World";

	ASSERT_EQ(sv.Compare(str2), -1);
	ASSERT_EQ(sv[1], 'e');

	char str3[] = "Hello";
	ASSERT_TRUE(sv == StringView(str3));
}
