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

//https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
bool dcclite::StrEndsWith(std::string_view str, std::string_view suffix) noexcept
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

std::string dcclite::GetSystemErrorMessage(const unsigned int error) noexcept
{
	return std::system_category().message(error);
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

#ifdef WIN32
//https://learn.microsoft.com/pt-br/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
//
//  
// Usage: SetThreadName ((DWORD)-1, "MainThread");  
//  

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR szName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  
void dcclite::SetThreadName(std::thread &thread, const char *threadName) 
{
	HANDLE h = thread.native_handle();

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = GetThreadId(h);
	info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)  
}

#else
void dcclite::SetThreadName(std::thread &thread, const char *threadName)
{
	//not implemented
}

#endif
