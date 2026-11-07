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
	#if (!defined assert)
		#define assert(x)
	#endif
#else
#include <assert.h>
#endif

#ifndef DCCLITE_VERSION
#define DCCLITE_VERSION "0.11.1"
#endif

namespace dcclite
{		
	constexpr unsigned char SERVO_DEFAULT_RANGE = 15;

	constexpr uint16_t DEFAULT_DCCLITE_SERVER_PORT = 8989;
	constexpr uint16_t DEFAULT_DCCPP_PORT = 2560;
	constexpr uint16_t DEFAULT_TERMINAL_SERVER_PORT = 4190;
	constexpr uint16_t DEFAULT_ZEROCONF_PORT = 9381;

	enum class DecoderTypes : uint8_t
	{
		DEC_NULL = 0,
		DEC_OUTPUT = 1,
		DEC_SENSOR = 2,
		DEC_SERVO_TURNOUT = 3,
		DEC_SIGNAL = 4,			//Only virtual, not implemented on Arduino
		DEC_TURNTABLE_AUTO_INVERTER = 5,
		DEC_QUAD_INVERTER = 6,
		DEC_BLINK_OUTPUT = 7,
		DEC_BLINK_OUTPUT_MASTER = 8
	};

	enum class DecoderStates
	{
		INACTIVE = 0,
		ACTIVE
	};

	inline DecoderStates operator!(const DecoderStates state) noexcept
	{
		return state == DecoderStates::ACTIVE ? DecoderStates::INACTIVE : DecoderStates::ACTIVE;
	}


	inline const char *DecoderStateName(const DecoderStates state) noexcept
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

		OUTD_RESERVED_BIT = 0x08,

		OUTD_USER_BIT4 = 0x10,
		OUTD_USER_BIT5 = 0x20,
		OUTD_USER_BIT6 = 0x40,

		OUTD_ACTIVE = 0x80,

		OUTD_USER_BITS = (OUTD_USER_BIT4 | OUTD_USER_BIT5 | OUTD_USER_BIT6)
	};

	enum BlinkOutputDecoderFlags : uint8_t
	{
		//make sure we use the same bits as OutputDecoderFlags
		BOUTD_INVERTED_OPERATION = OutputDecoderFlags::OUTD_INVERTED_OPERATION,
		BOUTD_IGNORE_SAVED_STATE = OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE,
		BOUTD_ACTIVATE_ON_POWER_UP = OutputDecoderFlags::OUTD_ACTIVATE_ON_POWER_UP,

		BOUTD_BLINKING = OUTD_USER_BIT4,
		BOUTD_OUTPUT_ACTIVE = OUTD_USER_BIT5,

		//the same as OutputDecoderFlags
		BOUTD_ACTIVE = OutputDecoderFlags::OUTD_ACTIVE
	};

	enum BlinkOutputDecoderControllerFlags : uint8_t
	{
		//make sure we use the same bits as OutputDecoderFlags

		//inverted makes no sense here
		BOUTMD_RESERVED0 = OutputDecoderFlags::OUTD_INVERTED_OPERATION,

		BOUTMD_IGNORE_SAVED_STATE = OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE,
		BOUTMD_ACTIVATE_ON_POWER_UP = OutputDecoderFlags::OUTD_ACTIVATE_ON_POWER_UP,

		BOUTMD_RESERVED1 = OutputDecoderFlags::OUTD_RESERVED_BIT,

		BOUTMD_ACTIVE = OutputDecoderFlags::OUTD_ACTIVE
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

		//runtime flags
		SNRD_DELAY = 0x10,
		SNRD_COOLDOWN = 0x20,
		SNRD_REMOTE_ACTIVE = 0x40,
		SNRD_ACTIVE = 0x80
	};

	enum TurntableAutoInverterDecoderFlags: uint8_t
	{
		TRTD_INVERTED = OUTD_INVERTED_OPERATION,		//Bit 0 - 0x01
		TRTD_RESERVED_BIT1 = OUTD_IGNORE_SAVED_STATE,	//IgnoreSaveState not used 0x02
		TRTD_REMOTE_ACTIVE = OUTD_ACTIVATE_ON_POWER_UP,	//Bit 2 - 0x04

		TRTD_RESERVED_BIT3 = OUTD_RESERVED_BIT,			//Bit 3 - 0x08

		TRTD_UNUSED_BIT4 = OUTD_USER_BIT4,				//Bit 4 - 0x10
		TRTD_UNUSED_BIT5 = OUTD_USER_BIT5,				//Bit 5 - 0x20
		TRTD_UNUSED_BIT6 = OUTD_USER_BIT6,				//Bit 6 - 0x40

		TRTD_ACTIVE = OUTD_ACTIVE						//Bit 7 - 0x80
	};

	enum QuadInverterDecoderFlags : uint8_t
	{		
		QUAD_INVERTED = OUTD_INVERTED_OPERATION,				//Bit 0 - 0x01
		QUAD_IGNORE_SAVED_STATE = OUTD_IGNORE_SAVED_STATE,		//Bit 1 - 0x02
		QUAD_ACTIVATE_ON_POWER_UP = OUTD_ACTIVATE_ON_POWER_UP,	//Bit 2 - 0x04

		QUAD_RESERVED = OUTD_RESERVED_BIT,						//Bit 3 - 0x08

		QUAD_UNUSED_BIT4 = OUTD_USER_BIT4,						//Bit 4 - 0x10
		QUAD_UNUSED_BIT5 = OUTD_USER_BIT5,						//Bit 5 - 0x20
		QUAD_UNUSED_BIT6 = OUTD_USER_BIT6,						//Bit 6 - 0x40

		QUAD_ACTIVE = OUTD_ACTIVE								//Bit 7 - 0x80
	};	

} //end of namespace dcclite
