#pragma once

#include <string_view>

namespace dcclite
{
	bool TryHexStrToBinary(std::uint8_t dest[], size_t destSize, std::string_view str);
}