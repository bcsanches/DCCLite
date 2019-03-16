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

#include <stdio.h>

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

			//NetClient::sendLog("finishTokendest", dest);
		}
	}

	Tokens Parser::GetToken(char *dest, unsigned int destSize)
	{
		unsigned int destPos = 0;

		m_iLastKnowPos = m_iPos;

		bool hexMode = false;

		for (;;)
		{
			char ch = m_pszCmd[m_iPos];
			if (ch)
			{
				++m_iPos;
			}
			else
			{
				break;
			}

			//NetClient::sendLog("parsing", "%c", ch);

			switch (ch)
			{
				case ' ':
				case '\n':
				case '\t':
				case '\r':
					break;

				case '<':
					return TOKEN_CMD_START;

				case '>':
					return TOKEN_CMD_END;

				case '.':
					return TOKEN_DOT;

				case ':':
					return TOKEN_COLON;

				case '0':
					ch = m_pszCmd[m_iPos];
					if ((ch == 'x') || (ch == 'X'))
					{
						//hex digit
						++m_iPos;
						ch = m_pszCmd[m_iPos];

						if (!ch)
							return TOKEN_ERROR;

						++m_iPos;

						hexMode = true;
					}
					else
					{
						ch = '0';
					}
					//fall throught

				default:
					if (IsDigit(ch) || (hexMode && IsHexLetter(ch)))
					{
						SafeCopy(dest, destPos, destSize, ch);					

						for (;;)
						{
							ch = m_pszCmd[m_iPos];
							if (IsDigit(ch) || (hexMode && IsHexLetter(ch)))
							{
								SafeCopy(dest, destPos, destSize, ch);
								++m_iPos;
							}
							else
							{
								FinishToken(dest, destPos, destSize);

								return hexMode ? TOKEN_HEX_NUMBER : TOKEN_NUMBER;
							}
						}
					}
					else if (IS_ID_START(ch))
					{
						SafeCopy(dest, destPos, destSize, ch);

						for (;;)
						{
							ch = m_pszCmd[m_iPos];

							if (IS_ID(ch))
							{
								SafeCopy(dest, destPos, destSize, ch);
								++m_iPos;
							}
							else
							{							
								FinishToken(dest, destPos, destSize);
								
								return TOKEN_ID;
							}
						}
					}

					return TOKEN_ERROR;
			}
		}

		return TOKEN_EOF;
	}

	Tokens Parser::GetNumber(int &dest)
	{
		char buffer[9];

		auto token = this->GetToken(buffer, sizeof(buffer));
		if ((token != TOKEN_NUMBER) && (token != TOKEN_HEX_NUMBER))
			return token;

		sscanf(buffer, (token == TOKEN_HEX_NUMBER) ? "%x" : "%d", &dest);		

		return TOKEN_NUMBER;
	}

	void Parser::PushToken()
	{
		m_iPos = m_iLastKnowPos;
	}
}
