// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Parser.h"

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

#include "Misc.h"

namespace dcclite
{

	Parser::Parser(StringView cmd) :
		m_svCmd(cmd)		
	{
		//empty
	}
		
	#define IS_ID_START(x) (((x >= 'a') && (x <= 'z')) || ((x >= 'A') && (x <= 'Z')) || (x == '_'))
	#define IS_ID(x) (IS_ID_START(x) || IsDigit(x))
	
	Token Parser::ParseId(const Tokens returnType)
	{			
		auto start = m_iPos;
		for (;;)
		{
			char ch = m_svCmd[m_iPos];
			if (IS_ID(ch))
			{
				++m_iPos;
				if (m_iPos == m_svCmd.GetSize())
					break;
			}
			else
			{
				break;
			}
		}

		return Token{ returnType, StringView{m_svCmd.GetData() + start, m_iPos - start} };
	}

	inline Token Parser::MakeSingleCharToken(Tokens type, unsigned int pos) const noexcept
	{
		return Token{ type, StringView{m_svCmd.GetData() + pos, 1 } };
	}

	Token Parser::GetToken(bool forceHexMode)
	{		
		for (;;)
		{
			if (m_iPos == m_svCmd.GetSize())
				break;
			
			char ch = m_svCmd[m_iPos++];

			if ((ch == ' ') || (ch == '\n') || (ch == '\t') || (ch == '\r'))
				continue;			

			switch (ch)
			{				
				case '<':
					return this->MakeSingleCharToken(Tokens::CMD_START, m_iPos - 1);

				case '>':
					return this->MakeSingleCharToken(Tokens::CMD_END, m_iPos - 1);

				case '.':
					return this->MakeSingleCharToken(Tokens::DOT, m_iPos - 1);

				case ':':
					return this->MakeSingleCharToken(Tokens::COLON, m_iPos - 1);

				case '#':
					return this->MakeSingleCharToken(Tokens::HASH, m_iPos - 1);

				case '/':
					return this->MakeSingleCharToken(Tokens::SLASH, m_iPos - 1);

				case '$':
					if (m_iPos == m_svCmd.GetSize())
						return Token{ Tokens::SYNTAX_ERROR, m_svCmd };

					//look ahead...
					ch = m_svCmd[m_iPos];
					if (!IS_ID_START(ch))
						return Token{ Tokens::SYNTAX_ERROR, m_svCmd };

					return this->ParseId(Tokens::VARIABLE_NAME);

				case '0':										
					if (m_iPos == m_svCmd.GetSize())
					{
						return this->MakeSingleCharToken(Tokens::NUMBER, m_iPos - 1);
					}

					//look ahead...
					ch = m_svCmd[m_iPos];
					if ((ch == 'x') || (ch == 'X'))
					{
						//hex digit - 0x or 0X
						++m_iPos;

						//we must have something after 0x
						if(m_iPos == m_svCmd.GetSize())
							return Token{ Tokens::SYNTAX_ERROR, m_svCmd };

						//first digit after 0x
						ch = m_svCmd[m_iPos];
						++m_iPos;

						forceHexMode = true;
					}
					else
					{
						ch = '0';
					}					
					[[fallthrough]];

				default:
					if (IsDigit(ch) || (forceHexMode && IsHexLetter(ch)))
					{						
						unsigned int startPos = m_iPos - 1;
						for (;;)
						{
							if (m_iPos == m_svCmd.GetSize())
							{
								return Token{ forceHexMode ? Tokens::HEX_NUMBER : Tokens::NUMBER , StringView{m_svCmd.GetData() + startPos, m_iPos - startPos} };
							}

							ch = m_svCmd[m_iPos];
							if (IsDigit(ch) || (forceHexMode && IsHexLetter(ch)))
							{								
								++m_iPos;
							}
							else
							{
								return Token{ forceHexMode ? Tokens::HEX_NUMBER : Tokens::NUMBER , StringView{m_svCmd.GetData() + startPos, m_iPos - startPos} };
							}
						}
					}
					else if (IS_ID_START(ch) && !forceHexMode)
					{
						--m_iPos;
						return this->ParseId(Tokens::ID);						
					}

					return Token{ Tokens::SYNTAX_ERROR, m_svCmd };
			}
		}

		return Token{ Tokens::END_OF_BUFFER, m_svCmd };
	}

	inline static int NumChar2Num(const char digit)
	{
		return (digit >= '0' && digit <= '9') ? digit - '0' : 0;
	}

	inline static int Char2Num(const char digit)
	{
		return (digit >= 'a') && (digit <= 'f') ? (digit - 'a') + 10 : 
			(digit >= 'A') && (digit <= 'F') ? (digit - 'A') + 10 : 
			NumChar2Num(digit);
	}

	static int Str2Num(StringView buffer, const bool hex)
	{
		int dest = 0;
		auto base = hex ? 16 : 10;

		for (unsigned i = 0; i < buffer.GetSize(); ++i)
		{
			dest = (dest * base) + Char2Num(buffer[i]);
		}

		return dest;
	}

	Tokens Parser::GetHexNumber(int &dest)
	{		
		auto token = this->GetToken(true);
		if (token.m_kToken != Tokens::HEX_NUMBER)
			return token.m_kToken;

		dest = Str2Num(token.m_svData, true);

		return Tokens::HEX_NUMBER;
	}


	Tokens Parser::GetNumber(int &dest)
	{
		auto token = this->GetToken();
		if ((token.m_kToken != Tokens::NUMBER) && (token.m_kToken != Tokens::HEX_NUMBER))
			return token.m_kToken;

		dest = Str2Num(token.m_svData, token.m_kToken == Tokens::HEX_NUMBER);

		return Tokens::NUMBER;
	}
}
