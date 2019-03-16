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
	enum Tokens
	{
		TOKEN_ID,
		TOKEN_NUMBER,
		TOKEN_HEX_NUMBER,

		TOKEN_CMD_START,
		TOKEN_CMD_END,

		TOKEN_DOT,

		TOKEN_COLON,

		TOKEN_EOF,

		TOKEN_ERROR
	};

	class Parser
	{
		private:
			const char *m_pszCmd;

			unsigned int m_iPos;
			unsigned int m_iLastKnowPos;

		public:
			Parser(const char *cmd);

			Tokens GetToken(char *dest, unsigned int destSize);
			Tokens GetNumber(int &dest);

			void PushToken();
	};
}

