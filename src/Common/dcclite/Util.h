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

#include <string>
#include <string_view>
#include <thread>

#define DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(X)	\
	X(const X &) = delete;						\
	X(X &&) = delete;							\
	X operator=(const X &) = delete;			\
	X &&operator=(X &&) = delete;

namespace dcclite
{
	bool TryHexStrToBinary(std::uint8_t dest[], size_t destSize, std::string_view str) noexcept;

	//
	//Try to parse a number, throws if string contains no number
	int ParseNumber(std::string_view str);

	/**
	* Returns a view to a string without leading and trailing white spaces
	* 
	* It may return an empty string: std::string_view("")
	
	*/
	std::string_view StrTrim(std::string_view str) noexcept;

	size_t StrCountLines(std::string_view str, size_t limit);

	bool StrEndsWith(std::string_view str, std::string_view suffix) noexcept;	

	std::string GetSystemLastErrorMessage() noexcept;

	std::string GetSystemErrorMessage(const unsigned int error) noexcept;
	

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Network helpers
	//
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	std::uint32_t h2nl(const std::uint32_t v) noexcept;
	std::uint16_t h2ns(const std::uint16_t v) noexcept;

	std::uint16_t n2hs(const std::uint16_t v) noexcept;
	std::uint32_t n2hl(const std::uint32_t v) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Thread helpers
	//
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetThreadName(std::thread &thread, const char *threadName);

	void SetMainThread();
	void ClearMainThread();

	bool IsMainThread();
}
