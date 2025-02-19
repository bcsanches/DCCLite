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
	inline bool IsDigit(char ch)
	{
		return ((ch >= '0') && (ch <= '9'));
	}

	inline bool IsHexLetter(char ch)
	{
		return ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F'));
	}

	inline bool IsHexDigit(char ch)
	{
		return IsDigit(ch) || IsHexLetter(ch);
	}

	inline int TryChar2Hex(char ch)
	{
		return ((ch >= '0') && (ch <= '9')) ? 0 + (ch - '0') : ((ch >= 'a') && (ch <= 'f')) ? 0 + (ch - 'a' + 10) : ((ch >= 'A') && (ch <= 'F')) ? 0 + (ch - 'A' + 10) : -1;
	}	

	int GetHeapFreeSpace();

	template <typename T>
	inline T MyMin(T a, T b) noexcept
	{
		return a < b ? a : b;
	}
}
