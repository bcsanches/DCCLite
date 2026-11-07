// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <vector>

#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>
#include <dcclite_shared/SharedLibDefs.h>

#include <exec/dcc/BlinkOutputDecoder.h>
#include <exec/dcc/BlinkOutputMasterDecoder.h>

#include <ArduinoLib.h>

#include <Storage.h>

#include <BlinkOutputDecoder.h>
#include <BlinkOutputMasterDecoder.h>

#include "MiniDuino.h"
#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;

namespace broker = dcclite::broker::exec::dcc;

using RemoteBlinkOutputDecoder = ::BlinkOutputDecoder;
using RemoteBlinkOutputMasterDecoder = ::BlinkOutputMasterDecoder;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

class DecoderManagerInstance;

template <typename T>
static dcclite::Packet WriteDecoderConfig(const char *json)
{
	Document document;
	document.Parse(json);

	dcclite::RName name{ document["name"].GetString() };
	auto address = broker::Address{ static_cast<uint16_t>(document["address"].GetUint()) };

	T output{
		address,
		name,
		g_DecoderServices,
		g_DeviceDecoderServices,
		document
	};

	dcclite::Packet packet;
	output.WriteConfig(packet);

	packet.Reset();

	return packet;
}


TEST(BlinkOutputMasterDecoder, InitialState)
{
	const char *json = R"JSON(
		[
			{
				"name": "TEST_BLINK_MASTER",
				"class": "BlinkOutputMaster",
				"address": 1301,
				"ignoreSavedState": false,
				"location": "TC",
				"slave":"BLINK"				
			},
			{
				"name": "BLINK",
				"class": "BlinkOutput",
				"address": 1302,
				"pin": 45,
				"inverted": false,
				"ignoreSavedState": false,
				"location": "TC",
				"blinkInterval": 100
			}
		]
	)JSON";	

	ManagedMiniDuino board;	

	board.Load(json);

	auto master = board.GetDecoder(0);
	ASSERT_EQ(master->GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER);

	auto slave = board.GetDecoder(1);
	ASSERT_EQ(slave->GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT);

	auto blinkSlave = dynamic_cast<BlinkOutputDecoder *>(slave);
	ASSERT_FALSE(blinkSlave->IsBlinking());
	
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	master->AcceptServerState(dcclite::DecoderStates::ACTIVE, millis());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
	
	ASSERT_TRUE(blinkSlave->IsBlinking());

	ArduinoLib::FixedTick(1);

	master->AcceptServerState(dcclite::DecoderStates::ACTIVE, millis());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	blinkSlave->AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	//make sure blink is blinking
	ArduinoLib::FixedTick(100);
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}
