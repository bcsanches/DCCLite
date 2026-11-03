#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "exec/dcc/BlinkOutputDecoder.h"

#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;

using namespace dcclite::broker::exec::dcc;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

TEST(BlinkOutputDecoder, Basic)
{
	const char *json = R"JSON(
		{
			"name": "TEST_BLINK",
			"class": "BlinkOutput",
			"address": 1301,
			"pin": 45,
			"inverted": false,
			"ignoreSavedState": false,
			"location": "TC",
			"blinkInterval": 100
		}
	)JSON";

	Document d;
	d.Parse(json);	


	BlinkOutputDecoder decoder{ Address{1301}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(decoder.GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT);

	dcclite::Packet packet;
	decoder.WriteConfig(packet);

	packet.Reset();

	ASSERT_EQ(dcclite::DecoderTypes::DEC_BLINK_OUTPUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 1301 }, Address{ packet });
	ASSERT_EQ(45, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(0, flags);

	auto interval = packet.Read<uint16_t>();
	ASSERT_EQ(100, interval);
}

TEST(BlinkOutputDecoder, IntervalZero)
{
	const char *json = R"JSON(
		{
			"name": "TEST_BLINK",
			"class": "BlinkOutput",
			"address": 1301,
			"pin": 45,
			"inverted": false,
			"ignoreSavedState": false,
			"location": "TC",
			"blinkInterval": 0
		}
	)JSON";

	Document d;
	d.Parse(json);

	EXPECT_THROW(BlinkOutputDecoder(Address { 1301 }, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d ), std::logic_error);	
}

TEST(BlinkOutputDecoder, IntervalUpperLimit)
{
	const char *json = R"JSON(
		{
			"name": "TEST_BLINK",
			"class": "BlinkOutput",
			"address": 1301,
			"pin": 45,
			"inverted": false,
			"ignoreSavedState": false,
			"location": "TC",
			"blinkInterval": 65536
		}
	)JSON";

	Document d;
	d.Parse(json);

	EXPECT_THROW(BlinkOutputDecoder(Address{ 1301 }, dcclite::RName{ "test" }, g_DecoderServices, g_DeviceDecoderServices, d), std::logic_error);
}
