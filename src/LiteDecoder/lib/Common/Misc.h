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
}
