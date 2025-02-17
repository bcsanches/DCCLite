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

	Parser::Parser(const char *cmd) :
		m_pszCmd(cmd),
		m_iPos(0),
		m_iLastKnowPos(0)
	{
		//empty
	}
		
	#define IS_ID_START(x) (((x >= 'a') && (x <= 'z')) || ((x >= 'A') && (x <= 'Z')) || (x == '_'))
	#define IS_ID(x) (IS_ID_START(x) || IsDigit(x))


	inline void SafeCopy(char *dest, unsigned int &destPos, unsigned int destSize, char ch)
	{
		if (destPos < destSize)
		{
			dest[destPos] = ch;
			++destPos;
		}
	}

	inline void FinishToken(char *dest, unsigned int &destPos, int destSize)
	{
		if (destSize > 0)
		{
			//if string is small, put a 0 on right position
			SafeCopy(dest, destPos, destSize, 0);

			//make sure string is closed if safeCopy does nothing
			dest[destSize - 1] = 0;			
		}
	}

	Tokens Parser::ParseId(char *dest, unsigned int destPos, const unsigned int destSize, const Tokens returnType)
	{				
		for (;;)
		{
			char ch = m_pszCmd[m_iPos];

			if (IS_ID(ch))
			{
				SafeCopy(dest, destPos, destSize, ch);
				++m_iPos;
			}
			else
			{
				FinishToken(dest, destPos, destSize);

				return returnType;
			}
		}
	}

	Tokens Parser::GetToken(char *dest, unsigned int destSize, bool forceHexMode)
	{
		unsigned int destPos = 0;

		m_iLastKnowPos = m_iPos;		

		for (;;)
		{
			for (;;)
			{
				char ch = m_pszCmd[m_iPos];

				if ((ch == ' ') || (ch == '\n') || (ch == '\t') || (ch == '\r'))
				{
					++m_iPos;
					continue;
				}

				break;
			}

			char ch = m_pszCmd[m_iPos];
			if (ch)
			{
				++m_iPos;
			}
			else
			{
				break;
			}			

			switch (ch)
			{				
				case '<':
					return Tokens::CMD_START;

				case '>':
					return Tokens::CMD_END;

				case '.':
					return Tokens::DOT;

				case ':':
					return Tokens::COLON;

				case '#':
					return Tokens::HASH;

				case '/':
					return Tokens::SLASH;

				case '$':
					ch = m_pszCmd[m_iPos];
					if (!IS_ID_START(ch))
						return Tokens::SYNTAX_ERROR;

					SafeCopy(dest, destPos, destSize, ch);

					++m_iPos;

					return this->ParseId(dest, destPos, destSize, Tokens::VARIABLE_NAME);

				case '0':
					ch = m_pszCmd[m_iPos];
					if ((ch == 'x') || (ch == 'X'))
					{
						//hex digit
						++m_iPos;
						ch = m_pszCmd[m_iPos];

						if (!ch)
							return Tokens::SYNTAX_ERROR;

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
						SafeCopy(dest, destPos, destSize, ch);					

						for (;;)
						{
							ch = m_pszCmd[m_iPos];
							if (IsDigit(ch) || (forceHexMode && IsHexLetter(ch)))
							{
								SafeCopy(dest, destPos, destSize, ch);
								++m_iPos;
							}
							else
							{
								FinishToken(dest, destPos, destSize);

								return forceHexMode ? Tokens::HEX_NUMBER : Tokens::NUMBER;
							}
						}
					}
					else if (IS_ID_START(ch))
					{
						SafeCopy(dest, destPos, destSize, ch);

						return this->ParseId(dest, destPos, destSize, Tokens::ID);						
					}

					return Tokens::SYNTAX_ERROR;
			}
		}

		return Tokens::END_OF_BUFFER;
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

	static int Str2Num(const char *buffer, const bool hex)
	{
		int dest = 0;
		auto base = hex ? 16 : 10;

		for (unsigned i = 0; buffer[i]; ++i)
		{
			dest = (dest * base) + Char2Num(buffer[i]);
		}

		return dest;
	}

	Tokens Parser::GetHexNumber(int &dest)
	{
		char buffer[9];
		auto token = this->GetToken(buffer, sizeof(buffer), true);
		if (token != Tokens::HEX_NUMBER)
			return token;

		dest = Str2Num(buffer, true);

		return Tokens::HEX_NUMBER;
	}


	Tokens Parser::GetNumber(int &dest)
	{
		char buffer[9];

		auto token = this->GetToken(buffer, sizeof(buffer));
		if ((token != Tokens::NUMBER) && (token != Tokens::HEX_NUMBER))
			return token;

		dest = Str2Num(buffer, token == Tokens::HEX_NUMBER);

		return Tokens::NUMBER;
	}

	void Parser::PushToken()
	{
		m_iPos = m_iLastKnowPos;
	}
}
