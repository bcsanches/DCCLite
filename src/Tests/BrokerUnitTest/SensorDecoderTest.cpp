#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "../TestsCommon/BrokerMockups.h"
#include "exec/dcc/SensorDecoder.h"

using namespace rapidjson;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

using namespace dcclite::broker::exec::dcc;

TEST(SensorDecoderTest, Basic)
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

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(0, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);
}

TEST(SensorDecoderTest, PullUpFlag)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":false,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);
}

TEST(SensorDecoderTest, PullUpAndInvertedFlag)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"activateDelay":0,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);
}

TEST(SensorDecoderTest, ActivateDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"activateDelay":3,
			"deactivateDelay":0,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 3000);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);
}

TEST(SensorDecoderTest, DeactivateDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"activateDelay":3,
			"deactivateDelay":4,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 3000);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 4000);
}

TEST(SensorDecoderTest, DelayMs)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"activateDelayMs":250,
			"deactivateDelayMs":350,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 250);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 350);
}

TEST(SensorDecoderTest, StartDelay)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"startDelay":5,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//startDelay
	ASSERT_EQ(packet.Read<uint16_t>(), 5000);
}

TEST(SensorDecoderTest, StartDelayMs)
{
	const char *json = R"JSON(
		{
			"name":"HLX_DTC00",
			"class":"Sensor",
			"pin":63,
			"address":"128",
			"pullup":true,
			"inverted":true,
			"startDelayMs":250,
			"location":"Helix",
			"comment":"Sensor infrared 00 - Entrada Staging A"
		}
	)JSON";

	Document d;
	d.Parse(json);

	SensorDecoder sensor{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, sensor.GetType());

	dcclite::Packet packet;

	sensor.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SENSOR, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(63, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(dcclite::SensorDecoderFlags::SNRD_PULL_UP | dcclite::SensorDecoderFlags::SNRD_INVERTED, flags);

	//activate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//deactivate delay
	ASSERT_EQ(packet.Read<uint16_t>(), 0);

	//startDelay
	ASSERT_EQ(packet.Read<uint16_t>(), 250);
}

