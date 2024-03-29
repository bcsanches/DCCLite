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

#include "Parser.h"

using namespace dcclite;

static void TestNum(const char *cmd, const int expectedNum)
{
	int num;
	Parser parser(cmd);

	EXPECT_EQ(parser.GetNumber(num), Tokens::NUMBER);
	EXPECT_EQ(num, expectedNum);
}

TEST(Parser, GetNumber)
{			
	TestNum("   5 ", 5);
	TestNum("10", 10);
	TestNum("0", 0);

	TestNum("255", 255);

	TestNum("255 5", 255);
}

TEST(Parser, GetNumber_HEX)
{
	TestNum("   0x5 ", 0x5);
	TestNum("0x10", 0x10);
	TestNum("0x0", 0x0);

	TestNum("0x255", 0x255);

	TestNum("0x255 5", 0x255);

	TestNum("0x1a", 0x1a);
	TestNum("0xfB", 0xfB);
	TestNum("0xfc", 0xfc);
	TestNum("0xFF", 0xFF);
}

TEST(Parser, GetVariable)
{
	char cmd[] = "   $VaRiA01_B   ";
	Parser parser(cmd);

	char dest[32];
	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::VARIABLE_NAME);

	ASSERT_STREQ(dest, "VaRiA01_B");
}

TEST(Parser, GetVariableErrors)
{	
	char dest[32];

	{
		Parser parser("$ Var");
		
		ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SYNTAX_ERROR);
	}

	{
		Parser parser("$0Var");

		ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SYNTAX_ERROR);
	}
	
	{
		Parser parser("$");

		ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SYNTAX_ERROR);
	}

	{
		Parser parser("$-as");

		ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SYNTAX_ERROR);
	}
}

static void TestHexNum(const char *cmd, const int expectedNum)
{
	int num;
	Parser parser(cmd);

	EXPECT_EQ(parser.GetHexNumber(num), Tokens::HEX_NUMBER);
	EXPECT_EQ(num, expectedNum);
}

TEST(Parser, GetHexNumber)
{
	TestHexNum("8B", 0x8B);
	TestHexNum("0x8B", 0x8B);	
}

TEST(Parser, GetId)
{
	char cmd[] = "   abc def _kij1a   ";
	Parser parser(cmd);

	char dest[32];
	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::ID);	

	ASSERT_STREQ(dest, "abc");

	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::ID);

	ASSERT_STREQ(dest, "def");

	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::ID);

	ASSERT_STREQ(dest, "_kij1a");
}

TEST(Parser, GetSlash)
{
	char cmd[] = "   /abc / ";
	Parser parser(cmd);

	char dest[32];
	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SLASH);	

	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::ID);

	ASSERT_STREQ(dest, "abc");

	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::SLASH);	

	ASSERT_EQ(parser.GetToken(dest, sizeof(dest)), Tokens::END_OF_BUFFER);
}
