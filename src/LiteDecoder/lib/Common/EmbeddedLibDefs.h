#pragma once

#include <stdint.h>

namespace dcclite
{
	typedef char PinType_t;
	constexpr char NULL_PIN = -1;

	enum class DecoderTypes : uint8_t
	{
		DEC_OUTPUT,
		DEC_INPUT
	};
}
