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

#include <stdint.h>

#if (!defined WIN32) && (!defined __linux__)
#define assert(x)
#else
#include <assert.h>
#endif

#ifndef DCCLITE_VERSION
#define DCCLITE_VERSION "0.5.0"
#endif

namespace dcclite
{		
	constexpr unsigned char SERVO_DEFAULT_RANGE = 15;

	enum class DecoderTypes : uint8_t
	{
		DEC_NULL = 0,
		DEC_OUTPUT = 1,
		DEC_SENSOR = 2,
		DEC_SERVO_TURNOUT = 3,
		DEC_SIGNAL = 4			//Only virtual, not implemented on Arduino
	};

	enum class DecoderStates
	{
		INACTIVE = 0,
		ACTIVE
	};

	inline const char *DecoderStateName(const DecoderStates state)
	{
		return state == DecoderStates::ACTIVE ? "ACTIVE" : "INACTIVE";
	}

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
		OUTD_IGNORE_SAVED_STATE = 0x02,
		OUTD_ACTIVATE_ON_POWER_UP = 0x04,

		OUTD_ACTIVE = 0x80
	};		

	enum ServoTurnoutDecoderFlags : uint8_t
	{
		SRVT_STATE_BITS = 0x03,

		SRVT_INVERTED_OPERATION = 0x04,
		SRVT_IGNORE_SAVED_STATE = 0x08,
		SRVT_ACTIVATE_ON_POWER_UP = 0x10,
		SRVT_INVERTED_FROG = 0x20,
		SRVT_INVERTED_POWER = 0x40,
		
		SRVT_POWER_ON = 0x80
	};

	enum SensorDecoderFlags : uint8_t
	{
		SNRD_PULL_UP = 0x01,
		SNRD_INVERTED = 0x02,

		SNRD_DELAY = 0x10,
		SNRD_COOLDOWN = 0x20,
		SNRD_REMOTE_ACTIVE = 0x40,
		SNRD_ACTIVE = 0x80
	};
} //end of namespace dcclite