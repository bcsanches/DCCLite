// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>
#include <dcclite_shared/SharedLibDefs.h>

#include <exec/dcc/SimpleOutputDecoder.h>

#include <ArduinoLib.h>

#include <OutputDecoder.h>
#include <Storage.h>

#include "MiniDuino.h"

#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;
namespace broker = dcclite::broker::exec::dcc;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

TEST(LiteDecoder, OutputDecoder_Default)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 45,
			"inverted": false,
			"ignoreSavedState": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));
	
	MiniDuino board;

	OutputDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	board.AddDecoder(remoteDecoder);

	ArduinoLib::FixedTick(1);

	ASSERT_FALSE(remoteDecoder.IsActive());

	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);	
}

TEST(LiteDecoder, OutputDecoder_RepeatedState)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 45,
			"inverted": false,
			"ignoreSavedState": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

	MiniDuino board;

	OutputDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	board.AddDecoder(remoteDecoder);

	ArduinoLib::FixedTick(1);

	ASSERT_FALSE(remoteDecoder.IsActive());

	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);
	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);
	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);
	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}

TEST(LiteDecoder, OutputDecoder_RepeatedStateInverted)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 45,
			"inverted": true,
			"ignoreSavedState": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

	MiniDuino board;

	OutputDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	board.AddDecoder(remoteDecoder);

	ArduinoLib::FixedTick(1);

	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);

	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);

	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
}

TEST(LiteDecoder, OutputDecoder_Inverted)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 45,
			"inverted": true,
			"ignoreSavedState": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

	MiniDuino board;

	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

	OutputDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

	board.AddDecoder(remoteDecoder);

	ArduinoLib::FixedTick(1);

	ASSERT_FALSE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
	

	remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

	ASSERT_TRUE(remoteDecoder.IsActive());
	ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}

TEST(LiteDecoder, OutputDecoder_SaveStateInverted)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 44,
			"inverted": true,
			"ignoreSavedState": false,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	{
		//read decoder type...
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ packet };

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		ArduinoLib::FixedTick(1);
	}	

	{		
		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };

		OutputDecoder remoteDecoder{ stream };

		//check  if state was loaded
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		ArduinoLib::FixedTick(1);
	}

	packet.Reset();

	{		
		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };
		OutputDecoder remoteDecoder{ stream };

		//check  if state was loaded
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);
	}
}


TEST(LiteDecoder, OutputDecoder_SaveState)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 44,
			"inverted": false,
			"ignoreSavedState": false,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	{
		//read decoder type...
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ packet };

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		ArduinoLib::FixedTick(1);
	}

	{
		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };

		OutputDecoder remoteDecoder{ stream };

		//check  if state was loaded
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		ArduinoLib::FixedTick(1);
	}

	packet.Reset();

	{
		MiniDuino board;

		Storage::EpromStream stream{ 0 };
		OutputDecoder remoteDecoder{ stream };

		//check  if state was loaded
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);
	}
}

TEST(LiteDecoder, OutputDecoder_IgnoreSaveState)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 44,
			"inverted": false,
			"ignoreSavedState": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	{
		//read decoder type...
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ packet };

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		board.AddDecoder(remoteDecoder);		

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		ArduinoLib::FixedTick(1);
	}

	{
		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };

		OutputDecoder remoteDecoder{ stream };

		//check  if state was ignored
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		ArduinoLib::FixedTick(1);
	}
}

TEST(LiteDecoder, OutputDecoder_ActivateOnPowerUpWithSaveStateFlag)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 44,
			"inverted": true,
			"ignoreSavedState": false,
			"activateOnPowerUp": false,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	{
		//read decoder type...
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ packet };

		//ActivateOnPowerUp has no effect with ignoreSavedState false
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);

		ArduinoLib::FixedTick(1);
	}

	{
		Storage::EpromStream stream{ 0 };

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ stream };

		//ActivateOnPowerUp has no effect with ignoreSavedState false
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);
	}
}

TEST(LiteDecoder, OutputDecoder_ActivateOnPowerUp)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 44,
			"inverted": false,
			"ignoreSavedState": true,
			"activateOnPowerUp": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	output.WriteConfig(packet);

	packet.Reset();

	{
		//read decoder type...
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ packet };

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);

		remoteDecoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, 1);

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);

		ArduinoLib::FixedTick(1);
	}

	{
		Storage::EpromStream stream{ 0 };

		MiniDuino board;

		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::LOW);

		OutputDecoder remoteDecoder{ stream };

		//ignored save state and activated
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(44), VoltageModes::HIGH);
	}
}

TEST(LiteDecoder, OutputDecoder_IgnoreSavedStateWithoutActivateOnPowerUp)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 46,
			"inverted": false,
			"ignoreSavedState": true,
			"activateOnPowerUp": false,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;
	output.WriteConfig(packet);
	packet.Reset();

	{
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;
		OutputDecoder remoteDecoder{ packet };

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(46), VoltageModes::LOW);

		board.AddDecoder(remoteDecoder);
		remoteDecoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, 1);

		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(46), VoltageModes::HIGH);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);
		ArduinoLib::FixedTick(1);
	}

	{
		MiniDuino board;
		Storage::EpromStream stream{ 0 };
		OutputDecoder remoteDecoder{ stream };

		// Saved state is ignored, and activateOnPowerUp is false.
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(46), VoltageModes::LOW);
	}
}

TEST(LiteDecoder, OutputDecoder_ActivateOnPowerUpWithSavedInactiveState)
{
	const char *json = R"JSON(
		{
			"name": "TC_SIG_11D_HG",
			"class": "Output",
			"address": 1301,
			"pin": 47,
			"inverted": false,
			"ignoreSavedState": false,
			"activateOnPowerUp": true,
			"location": "TC"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SimpleOutputDecoder output{ broker::Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;
	output.WriteConfig(packet);
	packet.Reset();

	{
		ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_OUTPUT));

		MiniDuino board;
		OutputDecoder remoteDecoder{ packet };

		// With ignoreSavedState=false, the saved state takes precedence over activateOnPowerUp.
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(47), VoltageModes::LOW);

		Storage::EpromStream stream{ 0 };
		remoteDecoder.SaveConfig(stream);
		ArduinoLib::FixedTick(1);
	}

	{
		MiniDuino board;
		Storage::EpromStream stream{ 0 };
		OutputDecoder remoteDecoder{ stream };

		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(47), VoltageModes::LOW);
	}
}
