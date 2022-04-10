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
#include "Parser.h"

#include <algorithm>

#ifndef WIN32
#include <dlfcn.h>
#include <netinet/in.h>
#include <string.h>
#else
#include <windows.h>
#include <winsock.h>
#endif

#include <stdexcept>
#include <sstream>


int dcclite::ParseNumber(const char *str)
{
	dcclite::Parser parser{ str };

	int adr;
	if (parser.GetNumber(adr) != dcclite::Tokens::NUMBER)
	{
		std::stringstream stream;

		stream << "[dcclite::ParseNumber] String " << str << " does not contains a valid number";

		throw std::runtime_error(stream.str());
	}

	return adr;
}

bool dcclite::TryHexStrToBinary(std::uint8_t dest[], size_t destSize, std::string_view str) noexcept
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

std::string_view dcclite::StrTrim(std::string_view str) noexcept
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

std::string dcclite::GetSystemErrorMessage(const unsigned int error) noexcept
{
#ifndef WIN32	

	return std::string(strerror(error));

#elif defined WIN32
	LPWSTR lpMsgBuf;
	auto numWChars = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf,
		0,
		NULL
	);

	if (!numWChars)
		return "GetSystemLastErrorMessage::FormatMessageW failed";

	auto requiredStrSize = WideCharToMultiByte(
		CP_UTF8,
		0,					// no flags
		lpMsgBuf,
		numWChars,
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (!requiredStrSize)
	{
		// Free the buffer.
		LocalFree(lpMsgBuf);

		return "GetSystemLastErrorMessage::WideCharToMultiByte failed";
	}

	std::string ret(requiredStrSize, '\0');

	WideCharToMultiByte(
		CP_UTF8,
		0,					// no flags
		lpMsgBuf,
		numWChars,
		&ret[0],
		requiredStrSize,
		nullptr,
		nullptr
	);

	// Free the buffer.
	LocalFree(lpMsgBuf);

	return ret;
#endif
}

std::string dcclite::GetSystemLastErrorMessage() noexcept
{
#ifndef WIN32
	return std::string{ GetSystemErrorMessage(errno) };
#elif defined WIN32
	return GetSystemErrorMessage(GetLastError());	
#endif
}

std::uint32_t dcclite::htonl(const std::uint32_t v) noexcept
{
	return ::htonl(v);
}

std::uint16_t dcclite::htons(const std::uint16_t v) noexcept
{
	return ::htons(v);
}

std::uint16_t dcclite::ntohs(const std::uint16_t v) noexcept
{
	return ::ntohs(v);
}

std::uint32_t dcclite::ntohl(const std::uint32_t v) noexcept
{
	return ::ntohl(v);
}
