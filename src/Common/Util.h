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

#include <string_view>

namespace dcclite
{
	bool TryHexStrToBinary(std::uint8_t dest[], size_t destSize, std::string_view str) noexcept;

	//
	//Try to parse a number, throws if string contains no number
	int ParseNumber(const char *str);

	/**
	* Returns a view to a string without leading and trailing white spaces
	* 
	* It may return an empty string: std::string_view("")
	
	*/
	std::string_view StrTrim(std::string_view str) noexcept;

	std::string GetSystemLastErrorMessage() noexcept;

	std::string GetSystemErrorMessage(const unsigned int error) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Network helpers
	//
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	std::uint32_t htonl(const std::uint32_t v) noexcept;
	std::uint16_t htons(const std::uint16_t v) noexcept;

	std::uint16_t ntohs(const std::uint16_t v) noexcept;
	std::uint32_t ntohl(const std::uint32_t v) noexcept;
}