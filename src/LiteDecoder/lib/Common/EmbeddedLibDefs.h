#pragma once

#include <cinttypes>

namespace dcclite
{
	typedef char PinType_t;
	constexpr char NULL_PIN = -1;

	enum class DecoderTypes : std::uint8_t
	{
		OUTPUT,
		INPUT
	};
}
