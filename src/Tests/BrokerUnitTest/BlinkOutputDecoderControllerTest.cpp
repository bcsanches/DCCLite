#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "exec/dcc/BlinkOutputDecoder.h"
#include "exec/dcc/BlinkOutputDecoderController.h"

#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;

using namespace dcclite::broker::exec::dcc;

TEST(BlinkOutputDecoderController, Basic)
{
	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	using namespace dcclite;

	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputController",
			"address": 1302,
			"ignoreSavedState": true,
			"activateOnPowerUp": true,			
			"location": "TC",
			"target":"TEST_BLINK"
		}
	)JSON";

	Document d;
	d.Parse(json);

	BlinkOutputDecoderController decoder{ Address{1302}, dcclite::RName{"testCtrl"}, decoderServices, deviceDecoderServices, d };
	ASSERT_EQ(decoder.GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT_CONTROLLER);
	
	const char *json2 = R"JSON(
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
	
	d.Parse(json2);
	BlinkOutputDecoder blinkDecoder{ Address{1301}, dcclite::RName{"TEST_BLINK"}, decoderServices, deviceDecoderServices, d };	

	deviceDecoderServices.RegisterDecoder(decoder);
	deviceDecoderServices.RegisterDecoder(blinkDecoder);

	decoder.InitAfterDeviceLoad();

	dcclite::Packet packet;
	decoder.WriteConfig(packet);

	packet.Reset();

	ASSERT_EQ(DecoderTypes::DEC_BLINK_OUTPUT_CONTROLLER, static_cast<DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 1302 }, Address{ packet });

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(BOUTDC_ACTIVATE_ON_POWER_UP | BOUTDC_IGNORE_SAVED_STATE, flags);

	//blinkDecoder index
	auto index = packet.Read<uint8_t>();
	ASSERT_EQ(1, index);
}

TEST(BlinkOutputDecoderController, NoTarget)
{
	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputController",
			"address": 1302,
			"ignoreSavedState": false,
			"location": "TC"					
		}
	)JSON";

	Document d;
	d.Parse(json);

	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	EXPECT_ANY_THROW(BlinkOutputDecoderController(Address{ 1301 }, dcclite::RName{ "test" }, decoderServices, deviceDecoderServices, d));
}

TEST(BlinkOutputDecoderController, NoTargetDecoder)
{
	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputController",
			"address": 1302,
			"ignoreSavedState": false,
			"location": "TC",
			"target":"TEST_BLINK"				
		}
	)JSON";

	Document d;
	d.Parse(json);

	BlinkOutputDecoderController decoder(Address{ 1301 }, dcclite::RName{ "test" }, decoderServices, deviceDecoderServices, d);

	EXPECT_THROW(decoder.InitAfterDeviceLoad(), std::exception);	
}
