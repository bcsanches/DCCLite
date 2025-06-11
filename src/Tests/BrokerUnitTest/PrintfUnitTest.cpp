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

#include <dcclite_shared/Printf.h>

class TestStream
{
	public:

		void Print(const char *str)
		{
			m_strStream << str;
		}

		void PrintFlash(const char *str)
		{
			m_strStream << str;
		}

		void Print(int n)
		{
			m_strStream << n;
		}

		void Print(unsigned n)
		{
			m_strStream << n;
		}

		void Print(char ch)
		{
			m_strStream << ch;			
		}

		bool IsEqual(const char *str)
		{
			return m_strStream.str().compare(str) == 0;			
		}
			
		std::stringstream m_strStream;
};

TEST(Printf, Hello)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Hello World" });

	EXPECT_TRUE(stream.IsEqual("Hello World"));	
}

TEST(Printf, EscapeSequence)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Esc seq \\n\\t\\r\\\"\\\\\\'\\" });

	EXPECT_TRUE(stream.IsEqual("Esc seq \n\t\r\"\\\'"));
}

TEST(Printf, CharData)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Char %c%c%c" }, 'Y', 'e', 's');

	EXPECT_TRUE(stream.IsEqual("Char Yes"));
}

TEST(Printf, IntData)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Int %d %d%d" }, 7, -12, 34);

	EXPECT_TRUE(stream.IsEqual("Int 7 -1234"));
}

TEST(Printf, UnsignedData)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Unsigned %u %u%u" }, 7, 12, 34);

	EXPECT_TRUE(stream.IsEqual("Unsigned 7 1234"));
}


TEST(Printf, StrData)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Hello %s" }, "world");

	EXPECT_TRUE(stream.IsEqual("Hello world"));
}

TEST(Printf, FlashStr)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "%z %s" }, "Hello", "world");

	EXPECT_TRUE(stream.IsEqual("Hello world"));
}


TEST(Printf, MultipleData)
{
	TestStream stream;

	dcclite::Printf(stream, dcclite::StringWrapper{ "Hello %s %d %c %u" }, "world", -25, 'A', 55);

	EXPECT_TRUE(stream.IsEqual("Hello world -25 A 55"));
}

