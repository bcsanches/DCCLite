#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "exec/dcc/BlinkOutputDecoder.h"
#include "exec/dcc/BlinkOutputMasterDecoder.h"

#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;

using namespace dcclite::broker::exec::dcc;

TEST(BlinkOutputMasterDecoder, Basic)
{
	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	using namespace dcclite;

	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputMaster",
			"address": 1302,
			"ignoreSavedState": true,
			"activateOnPowerUp": true,			
			"location": "TC",
			"slave":"TEST_BLINK"
		}
	)JSON";

	Document d;
	d.Parse(json);

	BlinkOutputMasterDecoder decoder{ Address{1302}, dcclite::RName{"testCtrl"}, decoderServices, deviceDecoderServices, d };
	ASSERT_EQ(decoder.GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER);
	
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

	ASSERT_EQ(DecoderTypes::DEC_BLINK_OUTPUT_MASTER, static_cast<DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 1302 }, Address{ packet });

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(BOUTMD_ACTIVATE_ON_POWER_UP | BOUTMD_IGNORE_SAVED_STATE, flags);

	//blinkDecoder index
	auto index = packet.Read<uint8_t>();
	ASSERT_EQ(1, index);
}

TEST(BlinkOutputMasterDecoder, NoTarget)
{
	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputMaster",
			"address": 1302,
			"ignoreSavedState": false,
			"location": "TC"					
		}
	)JSON";

	Document d;
	d.Parse(json);

	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	EXPECT_ANY_THROW(BlinkOutputMasterDecoder(Address{ 1301 }, dcclite::RName{ "test" }, decoderServices, deviceDecoderServices, d));
}

TEST(BlinkOutputMasterDecoder, NoTargetDecoder)
{
	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	const char *json = R"JSON(
		{
			"name": "TEST_BLINK_CONTROLLER",
			"class": "BlinkOutputMaster",
			"address": 1302,
			"ignoreSavedState": false,
			"location": "TC",
			"slave":"TEST_BLINK"				
		}
	)JSON";

	Document d;
	d.Parse(json);

	BlinkOutputMasterDecoder decoder(Address{ 1301 }, dcclite::RName{ "test" }, decoderServices, deviceDecoderServices, d);

	EXPECT_THROW(decoder.InitAfterDeviceLoad(), std::exception);	
}
