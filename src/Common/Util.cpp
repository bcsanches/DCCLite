// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Util.h"

#include "Misc.h"

#include <algorithm>

#ifndef WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif


bool dcclite::TryHexStrToBinary(std::uint8_t dest[], size_t destSize, std::string_view str)
{
	size_t dataIndex = 0;	

	for (auto it = str.begin(), end = str.end(); (it != end) && (dataIndex < destSize);)
	{
		it = std::find_if(it, end, dcclite::IsHexDigit);
		if (it == end)
			return false;
		
		char ch0 = *it;
		auto digit0 = TryChar2Hex(ch0);
		if (digit0 < 0)
			return false;

		++it;
		it = std::find_if(it, end, dcclite::IsHexDigit);
		if (it == end)
			return false;		

		char ch1 = *it;		
		auto digit1 = TryChar2Hex(ch1);
		if (digit1 < 0)
			return false;

		dest[dataIndex] = (digit0 << 4) | (digit1 & 0x0F);

		++dataIndex;
		++it;
	}

	return true;
}

std::string_view dcclite::StrTrim(std::string_view str)
{
	auto newBegin = str.begin();
	for (auto end = str.end(); (newBegin != end) && (*newBegin == ' '); ++newBegin);

	if (newBegin == str.end())
		return std::string_view("");

	auto newEnd = str.end() - 1;
	for (; (newEnd != newBegin) && (*newEnd == ' '); --newEnd);
	
	++newEnd;

	return str.substr(newBegin - str.begin(), newEnd - newBegin);
}

std::string dcclite::GetSystemLastErrorMessage()
{
#ifndef WIN32
	auto errorMsg = dlerror();

	return std::string(errorMsg);

#elif defined WIN32
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	std::string ret{ static_cast<char *>(lpMsgBuf) };

	// Free the buffer.
	LocalFree(lpMsgBuf);

	return ret;	
#endif
}
