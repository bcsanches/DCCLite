#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "BrokerMockups.h"
#include "exec/dcc/TurnoutDecoder.h"

using namespace rapidjson;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

using namespace dcclite::broker::exec::dcc;

TEST(ServoTurnoutDecoderTest, Basic)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,
			"inverted": false,
			"powerPin":26,
			"frogPin":28,
			"invertedFrog":false,
			"operationTime":1000,
			"range":10
		}
	)JSON";

	Document d;
	d.Parse(json);

	ServoTurnoutDecoder turnout{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SERVO_TURNOUT, turnout.GetType());

	dcclite::Packet packet;

	turnout.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SERVO_TURNOUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(3, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(0, flags);	

	//power pin
	ASSERT_EQ(26, packet.Read< dcclite::PinType_t>());

	//frog pin
	ASSERT_EQ(28, packet.Read< dcclite::PinType_t>());

	//start pos
	ASSERT_EQ(0, packet.Read< uint8_t>());

	//end pos
	ASSERT_EQ(10, packet.Read< uint8_t>());

	//ticks (operationTime / range)
	ASSERT_EQ(100, packet.Read< uint8_t>());
}

TEST(ServoTurnoutDecoderTest, StartAndEndPos)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,
			"inverted": false,
			"powerPin":26,
			"frogPin":28,
			"invertedFrog":false,
			"operationTime":1000,
			"startPos":10,
			"endPos":30
		}
	)JSON";

	Document d;
	d.Parse(json);

	ServoTurnoutDecoder turnout{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_SERVO_TURNOUT, turnout.GetType());

	dcclite::Packet packet;

	turnout.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SERVO_TURNOUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(3, packet.Read< dcclite::PinType_t>());

	auto flags = packet.Read<uint8_t>();
	ASSERT_EQ(0, flags);

	//power pin
	ASSERT_EQ(26, packet.Read< dcclite::PinType_t>());

	//frog pin
	ASSERT_EQ(28, packet.Read< dcclite::PinType_t>());

	//start pos
	ASSERT_EQ(10, packet.Read< uint8_t>());

	//end pos
	ASSERT_EQ(30, packet.Read< uint8_t>());

	//ticks (operationTime / range)
	ASSERT_EQ(50, packet.Read< uint8_t>());
}

void CheckTurnoutFlags(const char *json, uint8_t flags)
{
	Document d;
	d.Parse(json);

	ServoTurnoutDecoder turnout{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	turnout.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_SERVO_TURNOUT, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(Address{ 128 }, Address{ packet });
	ASSERT_EQ(3, packet.Read< dcclite::PinType_t>());	

	ASSERT_EQ(packet.Read<uint8_t>(), flags);

	ASSERT_EQ(flags, turnout.GetFlags());
}

TEST(ServoTurnoutDecoderTest, InvertedFlag)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,
			"inverted": true,
			"powerPin":26,
			"frogPin":28,
			"invertedFrog":false,
			"operationTime":1000,
			"range":10
		}
	)JSON";

	CheckTurnoutFlags(json, dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_OPERATION);
}

TEST(ServoTurnoutDecoderTest, IgnoreSaveStateFlag)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,			
			"ignoreSavedState":true,
			"powerPin":26,
			"frogPin":28,			
			"operationTime":1000,
			"range":10
		}
	)JSON";

	CheckTurnoutFlags(json, dcclite::ServoTurnoutDecoderFlags::SRVT_IGNORE_SAVED_STATE);
}

TEST(ServoTurnoutDecoderTest, ActivateOnPowerUpFlag)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,			
			"activateOnPowerUp":true,
			"powerPin":26,
			"frogPin":28,			
			"operationTime":1000,
			"range":10
		}
	)JSON";

	CheckTurnoutFlags(json, dcclite::ServoTurnoutDecoderFlags::SRVT_ACTIVATE_ON_POWER_UP);
}

TEST(ServoTurnoutDecoderTest, InvertedFrogFlag)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,						
			"powerPin":26,
			"frogPin":28,			
			"operationTime":1000,
			"range":10,
			"invertedFrog":true
		}
	)JSON";

	CheckTurnoutFlags(json, dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_FROG);
}

TEST(ServoTurnoutDecoderTest, InvertedPowerFlag)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,						
			"powerPin":26,
			"frogPin":28,			
			"operationTime":1000,
			"range":10,
			"invertedPower":true
		}
	)JSON";

	CheckTurnoutFlags(json, dcclite::ServoTurnoutDecoderFlags::SRVT_INVERTED_POWER);
}

TEST(ServoTurnoutDecoderTest, RemoteDecoder)
{
	const char *json = R"JSON(
		{
			"name":"STA_T01",
			"class":"ServoTurnout",
			"address":"1700",
			"pin": 3,						
			"powerPin":26,
			"frogPin":28,			
			"operationTime":1000,
			"range":10,
			"invertedPower":false
		}
	)JSON";

	Document d;
	d.Parse(json);

	ServoTurnoutDecoder turnout{ Address{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	dcclite::Packet packet;

	turnout.WriteConfig(packet);

	packet.Reset();

}

