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

#include <ArduinoLib.h>

#include <Console.h>
#include <Storage.h>
#include <SensorDecoder.h>

#include <dcclite_shared/Packet.h>

#include <exec/dcc/Address.h>
#include <exec/dcc/SensorDecoder.h>

#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;
namespace broker = dcclite::broker::exec::dcc;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

class MiniDuino
{
	static void Setup()
	{

	}

	static void Loop()
	{
		auto ticks = millis();

		for (auto dec : g_pclSingleton->m_vecDecoders)
		{
			dec->Update(ticks);
		}
	}

	public:
		MiniDuino()
		{
			assert(!g_pclSingleton);

			g_pclSingleton = this;

			ArduinoLib::Setup(Setup, Loop, "SensorDecoderTest");
		}

		~MiniDuino()
		{
			ArduinoLib::Finalize();

			g_pclSingleton = nullptr;
		}

		void AddDecoder(Decoder &dec)
		{
			m_vecDecoders.push_back(&dec);
		}

	private:
		std::vector<Decoder *> m_vecDecoders;

		static MiniDuino *g_pclSingleton;

};

MiniDuino *MiniDuino::g_pclSingleton = nullptr;

bool Storage::Custom_LoadModules(const Storage::Lump &lump, Storage::EpromStream &stream)
{
	return false;
}

bool Console::Custom_ParseCommand(dcclite::StringView command)
{
	return false;
}

void Storage::Custom_SaveModules(Storage::EpromStream &stream)
{

}

TEST(LiteDecoder, SensorTest_Default)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":false,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	SensorDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	{
		MiniDuino board;

		board.AddDecoder(remoteDecoder);
		
		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//cool down
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());
		
		ArduinoLib::FixedTick(30);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//detect change...goto cooldown state...
		ArduinoLib::FixedTick(1);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//now should change state
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());


	}
}

TEST(LiteDecoder, SensorTest_CoolDownFlip)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":false,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	SensorDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	{
		MiniDuino board;

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//cool down
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//same state
		ArduinoLib::FixedTick(10);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//pin goes low...
		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//should be on cooldown...
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//now should go back to initial state and ignore pin oscilation during cooldown
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());


	}
}

TEST(LiteDecoder, SensorTest_Inverted)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":true,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	SensorDecoder remoteDecoder{ packet };

	//Havent read pin yet...
	ASSERT_FALSE(remoteDecoder.IsActive());

	{
		MiniDuino board;

		board.AddDecoder(remoteDecoder);

		//first tick... goto cooldown mode...
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//cool down has finished...
		ArduinoLib::FixedTick(30);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//detect change, goto cool down
		ArduinoLib::FixedTick(1);		
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//cool down finished... 
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//detect change...goto cooldown state...
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//now should change state
		ArduinoLib::FixedTick(30);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());

	}
}

TEST(LiteDecoder, SensorTest_StartDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":false,
			"startDelay":2,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);	

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	{
		MiniDuino board;			

		SensorDecoder remoteDecoder{ packet };

		ASSERT_FALSE(remoteDecoder.IsActive());

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//cool down and start delay...
		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		//still on start delay...
		ArduinoLib::FixedTick(49);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//should finish start delay and detect state change
		ArduinoLib::FixedTick(2000);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//cooldown state now
		ArduinoLib::FixedTick(1);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());	

		//should now get state change
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsDelayActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsActive());

		//
		//go up again and see if delay is not reused...

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//now should change state
		ArduinoLib::FixedTick(30);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());
	}
}

TEST(LiteDecoder, SensorTest_ActivateDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":false,
			"startDelay":0,
			"activateDelayMs":500,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	SensorDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	{
		MiniDuino board;

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//cool down and start delay...
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//cool down is over, but now on delay...
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//should be on same state....
		ArduinoLib::FixedTick(200);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//now should have changed state...
		ArduinoLib::FixedTick(300);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//same state...
		ArduinoLib::FixedTick(300);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//cooldown state now
		ArduinoLib::FixedTick(1);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//should now get state change
		ArduinoLib::FixedTick(30);
		ASSERT_FALSE(remoteDecoder.IsDelayActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsActive());		
	}
}


TEST(LiteDecoder, SensorTest_DeactivateDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":false,
			"inverted":false,
			"startDelay":0,
			"activateDelayMs":0,
			"deactivateDelay":1,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	broker::SensorDecoder sensor{ broker::Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();

	//read decoder type...
	packet.Read<uint8_t>();

	SensorDecoder remoteDecoder{ packet };

	ASSERT_FALSE(remoteDecoder.IsActive());

	{
		MiniDuino board;

		board.AddDecoder(remoteDecoder);

		ArduinoLib::FixedTick(1);

		ASSERT_FALSE(remoteDecoder.IsActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::HIGH);

		//cool down
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());

		//cool down is over... 
		ArduinoLib::FixedTick(30);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//same state...
		ArduinoLib::FixedTick(300);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		ArduinoLib::SetPinDigitalVoltage(63, VoltageModes::LOW);

		//cooldown state now
		ArduinoLib::FixedTick(1);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//same state
		ArduinoLib::FixedTick(15);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_TRUE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());

		//cool down is over, now on delay
		ArduinoLib::FixedTick(15);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//same state....
		ArduinoLib::FixedTick(15);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//same state....
		ArduinoLib::FixedTick(983);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//same state....
		ArduinoLib::FixedTick(1);
		ASSERT_TRUE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_TRUE(remoteDecoder.IsDelayActive());

		//should now get state change
		ArduinoLib::FixedTick(1);
		ASSERT_FALSE(remoteDecoder.IsActive());
		ASSERT_FALSE(remoteDecoder.IsCoolDownActive());
		ASSERT_FALSE(remoteDecoder.IsDelayActive());		
	}
}