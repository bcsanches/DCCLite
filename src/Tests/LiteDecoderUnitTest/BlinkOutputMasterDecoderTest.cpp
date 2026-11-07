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

namespace DecoderManager
{
	std::vector<Decoder *> g_vecDecoders;

	DecoderManagerInstance *g_pclInstance = nullptr;

	Decoder *TryGetDecoder(const uint8_t slot) noexcept
	{
		assert(g_pclInstance);

		return g_vecDecoders.at(slot);
	}

	void Clear()
	{
		g_vecDecoders.clear();
	}

	void RegisterDecoder(Decoder &decoder)
	{
		g_vecDecoders.push_back(&decoder);
	}
}

class DecoderManagerInstance
{
	public:
		DecoderManagerInstance()
		{
			assert(DecoderManager::g_pclInstance == nullptr);
			assert(DecoderManager::g_vecDecoders.empty());

			DecoderManager::g_pclInstance = this;
		}

		~DecoderManagerInstance()
		{
			DecoderManager::g_vecDecoders.clear();

			DecoderManager::g_pclInstance = nullptr;
		}
};

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
		{
			"name": "TEST_BLINK_CTRL",
			"class": "BlinkOutputController",
			"address": 1301,			
			"ignoreSavedState": false,
			"location": "TC",
			"blinkInterval": 100
		}
	)JSON";

	auto packet = WriteDecoderConfig<broker::BlinkOutputMasterDecoder>(json);	

	ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER));

	MiniDuino board;
	RemoteBlinkOutputMasterDecoder masterDecoder{ packet };	

	RemoteBlinkOutputDecoder decoder{ packet };

	ASSERT_EQ(decoder.GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT);

	ASSERT_FALSE(decoder.IsInverted());
	ASSERT_FALSE(decoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, millis());

	ASSERT_TRUE(decoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
}
