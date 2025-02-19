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

#include "StringView.h"

namespace dcclite
{
	enum class Tokens
	{	
		CMD_START = '<',
		CMD_END = '>',

		DOT = '.',

		COLON = ':',		

		HASH = '#',

		SLASH = '/',

		VARIABLE_NAME = 256,

		ID,
		NUMBER,
		HEX_NUMBER,

		END_OF_BUFFER,

		SYNTAX_ERROR
	};

	struct Token
	{
		inline Token(Tokens tk, StringView view):
			m_svData{ view },
			m_kToken{ tk }
		{
			//empty
		}

		StringView	m_svData;
		Tokens		m_kToken;
	};

	class Parser
	{
		private:
			StringView m_svCmd;

			unsigned int m_iPos;
			unsigned int m_iLastKnowPos;

			[[nodiscard]] inline Token MakeSingleCharToken(Tokens type, unsigned int pos) const noexcept;

			[[nodiscard]] Token ParseId(const Tokens returnType);

		public:
			explicit Parser(StringView cmd);

			[[nodiscard]] Token GetToken(bool forceHexMode = false);
			[[nodiscard]] Tokens GetNumber(int &dest);
			[[nodiscard]] Tokens GetHexNumber(int &dest);

			void PushToken();
	};
}

