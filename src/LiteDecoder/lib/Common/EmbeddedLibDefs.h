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

	/**
	
	//https://github.com/DccPlusPlus/BaseStation/wiki/Commands-for-DCCpp-BaseStation
	IFLAG, bit 0: 0 = forward operation (ACTIVE=HIGH / INACTIVE=LOW)
			  1 = inverted operation (ACTIVE=LOW / INACTIVE=HIGH)

	IFLAG, bit 1: 0 = state of pin restored on power-up to either ACTIVE or INACTIVE
					  depending on state before power-down.
				  1 = state of pin set on power-up, or when first created,
					  to either ACTIVE of INACTIVE depending on IFLAG, bit 2

	IFLAG, bit 2: 0 = state of pin set to INACTIVE uponm power-up or when first created
				  1 = state of pin set to ACTIVE uponm power-up or when first created

	
	*/
	enum OutputDecoderFlags : uint8_t
	{
		OUTD_INVERTED_OPERATION = 0x01,
		OUTD_SET_ON_POWER_UP = 0x02,
		OUTD_ACTIVATE_ON_POWER_UP = 0x04
	};
}