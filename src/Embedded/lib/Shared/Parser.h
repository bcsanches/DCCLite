// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

namespace dcclite
{
	enum class Tokens
	{
		ID,
		NUMBER,
		HEX_NUMBER,

		CMD_START,
		CMD_END,

		DOT,

		COLON,

		END_OF_BUFFER,

		HASH,

		SYNTAX_ERROR
	};

	class Parser
	{
		private:
			const char *m_pszCmd;

			unsigned int m_iPos;
			unsigned int m_iLastKnowPos;	

			void SkipBlanks();

		public:
			explicit Parser(const char *cmd);

			Tokens GetToken(char *dest, unsigned int destSize, bool forceHexMode = false);
			Tokens GetNumber(int &dest);
			Tokens GetHexNumber(int &dest);

			void PushToken();
	};
}

