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

		VARIABLE_NAME,

		SLASH,

		SYNTAX_ERROR
	};

	class Parser
	{
		private:
			const char *m_pszCmd;

			unsigned int m_iPos;
			unsigned int m_iLastKnowPos;	

			void SkipBlanks();

			[[nodiscard]] Tokens ParseId(char *dest, unsigned int destPos, const unsigned int destSize, const Tokens returnType);

		public:
			explicit Parser(const char *cmd);

			[[nodiscard]] Tokens GetToken(char *dest, const unsigned int destSize, bool forceHexMode = false);
			[[nodiscard]] Tokens GetNumber(int &dest);
			[[nodiscard]] Tokens GetHexNumber(int &dest);

			void PushToken();
	};
}

