#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "BrokerMockups.h"
#include "dcc/SimpleOutputDecoder.h"

using namespace rapidjson;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

using namespace dcclite::broker;

TEST(SimpleOutputDecoderTest, Basic)
{
	const char *json = R"JSON(
		{
			"name": "ST_PNL_EXTC_LED_TRK_07",
			"class": "Output",
			"address": "4467",
			"pin": 64,
			"inverted": false,
			"ignoreSavedState": false
		}
	)JSON";

	Document d;
	d.Parse(json);

	SimpleOutputDecoder decoder{ DccAddress{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_OUTPUT, decoder.GetType());

	dcclite::Packet packet;

	decoder.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_OUTPUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(DccAddress{ 128 }, DccAddress{ packet });
	ASSERT_EQ(64, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(0, flags);
}

static void CheckFlags(const char *json, uint8_t flags)
{
	Document d;
	d.Parse(json);

	SimpleOutputDecoder decoder{ DccAddress{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	decoder.WriteConfig(packet);

	if (flags & dcclite::OutputDecoderFlags::OUTD_INVERTED_OPERATION)
	{
		ASSERT_TRUE(decoder.InvertedOperation());
	}

	if (flags & dcclite::OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE)
	{
		ASSERT_TRUE(decoder.IgnoreSavedState());
	}

	if (flags & dcclite::OutputDecoderFlags::OUTD_ACTIVATE_ON_POWER_UP)
	{
		ASSERT_TRUE(decoder.ActivateOnPowerUp());
	}

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_OUTPUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(DccAddress{ 128 }, DccAddress{ packet });
	ASSERT_EQ(64, packet.Read< dcclite::PinType_t>());

	ASSERT_EQ(packet.Read<uint8_t>(), flags);	
}

TEST(SimpleOutputDecoderTest, InvertedFlag)
{
	const char *json = R"JSON(
		{
			"name": "ST_PNL_EXTC_LED_TRK_07",
			"class": "Output",
			"address": "4467",
			"pin": 64,
			"inverted": true,
			"ignoreSavedState": false
		}
	)JSON";

	CheckFlags(json, dcclite::OutputDecoderFlags::OUTD_INVERTED_OPERATION);
}

TEST(SimpleOutputDecoderTest, IgnoreSaveStateFlag)
{
	const char *json = R"JSON(
		{
			"name": "ST_PNL_EXTC_LED_TRK_07",
			"class": "Output",
			"address": "4467",
			"pin": 64,
			"inverted": false,
			"ignoreSavedState": true
		}
	)JSON";

	CheckFlags(json, dcclite::OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE);
}

TEST(SimpleOutputDecoderTest, ActivateOnPowerUp)
{
	const char *json = R"JSON(
		{
			"name": "ST_PNL_EXTC_LED_TRK_07",
			"class": "Output",
			"address": "4467",
			"pin": 64,
			"inverted": false,
			"ignoreSavedState": false,
			"activateOnPowerUp": true
		}
	)JSON";

	CheckFlags(json, dcclite::OutputDecoderFlags::OUTD_ACTIVATE_ON_POWER_UP);
}
