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

#include <dcclite_shared/Parser.h>

using namespace dcclite;

static void TestNum(const char *cmd, const int expectedNum)
{
	int num;
	Parser parser(StringView{ cmd });

	EXPECT_EQ(parser.GetNumber(num), Tokens::NUMBER);
	EXPECT_EQ(num, expectedNum);
}

TEST(Parser, GetNumber)
{			
	TestNum("   5 ", 5);
	TestNum("10", 10);

	//This one is special, as it checks for a special case inside the parser while trying to detect hex numbers
	TestNum("0", 0);

	TestNum("3", 3);

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
	Parser parser(StringView{ cmd });

	auto token = parser.GetToken();
	ASSERT_EQ(token.m_kToken, Tokens::VARIABLE_NAME);

	ASSERT_TRUE(token.m_svData.Compare("VaRiA01_B") == 0);
}

TEST(Parser, GetVariableErrors)
{		
	{
		Parser parser(StringView{ "$ Var" });
		
		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	}

	{
		Parser parser(StringView{ "$0Var" });

		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	}
	
	{
		Parser parser(StringView{ "$" });

		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	}

	{
		Parser parser(StringView{ "$-as" });

		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	}
}

static void TestHexNum(const char *cmd, const int expectedNum)
{
	int num;
	Parser parser(StringView{ cmd });

	EXPECT_EQ(parser.GetHexNumber(num), Tokens::HEX_NUMBER);
	EXPECT_EQ(num, expectedNum);
}

TEST(Parser, GetHexNumber)
{
	TestHexNum("8B", 0x8B);
	TestHexNum("0x8B", 0x8B);	
}

TEST(Parser, HexErrors)
{
	ASSERT_EQ(Parser{ StringView{ "0x" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	ASSERT_EQ(Parser{ StringView{ "0X" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	ASSERT_EQ(Parser{ StringView{ "0x 5" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	ASSERT_EQ(Parser{ StringView{ "0x abc" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);

	ASSERT_EQ(Parser{ StringView{ "0x_abc" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
	ASSERT_EQ(Parser{ StringView{ "0xZabc" } }.GetToken().m_kToken, Tokens::SYNTAX_ERROR);
}

TEST(Parser, GetId)
{
	char cmd[] = "   abc def _kij1a   ";
	Parser parser(StringView{ cmd });

	auto token = parser.GetToken();
	ASSERT_EQ(token.m_kToken, Tokens::ID);
	ASSERT_TRUE(token.m_svData.Compare("abc") == 0);

	token = parser.GetToken();
	ASSERT_EQ(token.m_kToken, Tokens::ID);
	ASSERT_TRUE(token.m_svData.Compare("def") == 0);

	token = parser.GetToken();	
	ASSERT_EQ(token.m_kToken, Tokens::ID);
	ASSERT_TRUE(token.m_svData.Compare("_kij1a") == 0);	
}

TEST(Parser, GetSlash)
{
	char cmd[] = "   /abc / ";
	Parser parser(StringView{ cmd });
	
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SLASH);	

	auto token = parser.GetToken();
	ASSERT_EQ(token.m_kToken, Tokens::ID);

	ASSERT_TRUE(token.m_svData.Compare("abc") == 0);

	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SLASH);	

	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::END_OF_BUFFER);
}

TEST(Parser, SingleCharTokens)
{		
	Parser parser{ StringView{ "<>.:#/$a a" } };

	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::CMD_START);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::CMD_END);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::DOT);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::COLON);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::HASH);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::SLASH);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::VARIABLE_NAME);
	ASSERT_EQ(parser.GetToken().m_kToken, Tokens::ID);
}

TEST(Parser, SpecialCases)
{
	{
		//empty string
		Parser parser(StringView{ "", 0 });

		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::END_OF_BUFFER);
	}

	{
		//only blanks...
		Parser parser(StringView{ "   \n\t\r   \t"});

		ASSERT_EQ(parser.GetToken().m_kToken, Tokens::END_OF_BUFFER);
	}




}