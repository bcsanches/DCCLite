// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <dcclite_shared/Packet.h>
#include <dcclite_shared/SharedLibDefs.h>

#include <ArduinoLib.h>

#include <OutputDecoderState.h>

#include "MiniDuino.h"

constexpr dcclite::PinType_t TEST_PIN = 45;

static void WriteOutputDecoderStatePacket(dcclite::Packet &packet, uint8_t flags)
{
	packet.Write8(TEST_PIN);
	packet.Write8(flags);
	packet.Reset();
}
TEST(LiteDecoder, OutputDecoderState_UserBitsInitiallyEmpty)
{
	MiniDuino board;
	dcclite::Packet packet;
	WriteOutputDecoderStatePacket(packet, dcclite::OUTD_IGNORE_SAVED_STATE);

	OutputDecoderState state{ packet };

	ASSERT_EQ(state.GetUserBits(), 0);
}

TEST(LiteDecoder, OutputDecoderState_SetAndGetUserBits)
{
	MiniDuino board;
	const auto userBits = static_cast<uint8_t>(dcclite::BOUTD_BLINKING | dcclite::BOUTD_OUTPUT_ACTIVE);
	const auto flags = static_cast<uint8_t>(dcclite::OUTD_IGNORE_SAVED_STATE | dcclite::OUTD_INVERTED_OPERATION | userBits);

	dcclite::Packet packet;
	WriteOutputDecoderStatePacket(packet, flags);

	OutputDecoderState state{ packet };

	ASSERT_EQ(state.GetUserBits(), userBits);

	const auto replacementBits = static_cast<uint8_t>(dcclite::BOUTD_BLINKING);
	state.SetUserBits(replacementBits);

	ASSERT_EQ(state.GetUserBits(), replacementBits);
	ASSERT_TRUE(state.IsInverted());
	ASSERT_TRUE(state.IgnoreSavedState());
}

TEST(LiteDecoder, OutputDecoderState_SetUserBitsMasksNonUserFlags)
{
	MiniDuino board;
	dcclite::Packet packet;
	WriteOutputDecoderStatePacket(packet, dcclite::OUTD_IGNORE_SAVED_STATE);

	OutputDecoderState state{ packet };
	const auto requestedBits = static_cast<uint8_t>(dcclite::OUTD_ACTIVE | dcclite::OUTD_INVERTED_OPERATION | dcclite::BOUTD_BLINKING);

	state.SetUserBits(requestedBits);

	ASSERT_EQ(state.GetUserBits(), dcclite::BOUTD_BLINKING);
	ASSERT_FALSE(state.IsActive());
	ASSERT_FALSE(state.IsInverted());
}

TEST(LiteDecoder, OutputDecoderState_SetActiveReturnsTrueWhenStateChanges)
{
	MiniDuino board;
	dcclite::Packet packet;
	WriteOutputDecoderStatePacket(packet, dcclite::OUTD_IGNORE_SAVED_STATE);

	OutputDecoderState state{ packet };

	ASSERT_FALSE(state.IsActive());

	ASSERT_TRUE(state.SetActive(dcclite::DecoderStates::ACTIVE));
	ASSERT_TRUE(state.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(TEST_PIN), VoltageModes::HIGH);

	ASSERT_TRUE(state.SetActive(dcclite::DecoderStates::INACTIVE));
	ASSERT_FALSE(state.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(TEST_PIN), VoltageModes::LOW);
}

TEST(LiteDecoder, OutputDecoderState_SetActiveReturnsFalseWhenStateIsUnchanged)
{
	MiniDuino board;
	dcclite::Packet packet;
	WriteOutputDecoderStatePacket(packet, dcclite::OUTD_IGNORE_SAVED_STATE);

	OutputDecoderState state{ packet };

	ASSERT_FALSE(state.SetActive(dcclite::DecoderStates::INACTIVE));
	ASSERT_FALSE(state.IsActive());

	ASSERT_TRUE(state.SetActive(dcclite::DecoderStates::ACTIVE));
	ASSERT_FALSE(state.SetActive(dcclite::DecoderStates::ACTIVE));
	ASSERT_TRUE(state.IsActive());

	ASSERT_TRUE(state.SetActive(dcclite::DecoderStates::INACTIVE));
	ASSERT_FALSE(state.SetActive(dcclite::DecoderStates::INACTIVE));
	ASSERT_FALSE(state.IsActive());
}
