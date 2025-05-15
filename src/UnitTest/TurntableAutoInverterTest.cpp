#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <dcclite_shared/Packet.h>

#include "BrokerMockups.h"
#include "exec/SensorDecoder.h"
#include "exec/TurntableAutoInverterDecoder.h"

using namespace rapidjson;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

using namespace dcclite::broker;

TEST(TurntableAutoInverterDecoderTest, Basic)
{
	const char *json = R"JSON(
		[
			{
				"name":"HLX_DTC00",
				"class":"Sensor",
				"pin":63,
				"address":"128"
			},
			{
				"name":"HLX_DTC01",
				"class":"Sensor",
				"pin":64,
				"address":"129"
			},
			{
				"name": "ST_TURNTABLE",
				"class": "TurntableAutoInverter",
				"address": "4467",
				"sensorA": "HLX_DTC00",			
				"sensorB": "HLX_DTC01",
				"trackPowerAPin":10,			
				"trackPowerBPin":11
			}
		]
	)JSON";

	Document d;
	d.Parse(json);

	auto ar = d.GetArray();

	TurntableAutoInverterDecoder decoder{ DccAddress{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, ar[2]};

	ASSERT_EQ(dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER, decoder.GetType());

	dcclite::Packet packet;

	decoder.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(DccAddress{ 128 }, DccAddress{ packet });
	ASSERT_EQ(0, packet.Read<uint8_t>());												//reserved - always zero
	ASSERT_EQ(TURNTABLE_AUTO_INVERTER_DEFAULT_FLIP_INTERVAL, packet.Read<uint8_t>());
	ASSERT_EQ(0, packet.Read<uint8_t>());	//sensorA Index
	ASSERT_EQ(0, packet.Read<uint8_t>());	//sensorB Index
	ASSERT_EQ(10, packet.Read<dcclite::PinType_t>());
	ASSERT_EQ(11, packet.Read<dcclite::PinType_t>());

	//
	//Instantiate sensors	
	SensorDecoder a0{ DccAddress{128}, dcclite::RName{"HLX_DTC00"}, g_DecoderServices, g_DeviceDecoderServices, ar[0] };
	SensorDecoder a1{ DccAddress{129}, dcclite::RName{"HLX_DTC01"}, g_DecoderServices, g_DeviceDecoderServices, ar[1] };

	//not necessary to register, but we do to make sure a0 and a1 does not use index 0, to make sure the decoder retrieved the index
	g_DeviceDecoderServices.RegisterDecoder(decoder);

	g_DeviceDecoderServices.RegisterDecoder(a0);
	g_DeviceDecoderServices.RegisterDecoder(a1);

	//
	//It will lookup sensors
	decoder.InitAfterDeviceLoad();

	packet.Reset();
	decoder.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(DccAddress{ 128 }, DccAddress{ packet });
	ASSERT_EQ(0, packet.Read<uint8_t>());												//reserved - always zero
	ASSERT_EQ(TURNTABLE_AUTO_INVERTER_DEFAULT_FLIP_INTERVAL, packet.Read<uint8_t>());
	ASSERT_EQ(1, packet.Read<uint8_t>());	//sensorA Index
	ASSERT_EQ(2, packet.Read<uint8_t>());	//sensorB Index
	ASSERT_EQ(10, packet.Read<dcclite::PinType_t>());
	ASSERT_EQ(11, packet.Read<dcclite::PinType_t>());
}

TEST(TurntableAutoInverterDecoderTest, CustomFlipInterval)
{
	const char *json = R"JSON(				
			{
				"name": "ST_TURNTABLE",
				"class": "TurntableAutoInverter",
				"address": "4467",
				"sensorA": "HLX_DTC00",			
				"sensorB": "HLX_DTC01",
				"trackPowerAPin":10,			
				"trackPowerBPin":11,
				"flipInterval":20
			}		
	)JSON";

	Document d;
	d.Parse(json);	

	TurntableAutoInverterDecoder decoder{ DccAddress{128}, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d };

	ASSERT_EQ(dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER, decoder.GetType());

	dcclite::Packet packet;

	decoder.WriteConfig(packet);

	packet.Reset();
	ASSERT_EQ(dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER, static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()));
	ASSERT_EQ(DccAddress{ 128 }, DccAddress{ packet });
	ASSERT_EQ(0, packet.Read<uint8_t>());												//reserved - always zero
	ASSERT_EQ(20, packet.Read<uint8_t>());
	ASSERT_EQ(0, packet.Read<uint8_t>());	//sensorA Index
	ASSERT_EQ(0, packet.Read<uint8_t>());	//sensorB Index
	ASSERT_EQ(10, packet.Read<dcclite::PinType_t>());
	ASSERT_EQ(11, packet.Read<dcclite::PinType_t>());
}

TEST(TurntableAutoInverterDecoderTest, DuplicatedSensorName)
{
	const char *json = R"JSON(				
			{
				"name": "ST_TURNTABLE",
				"class": "TurntableAutoInverter",
				"address": "4467",
				"sensorA": "HLX_DTC00",			
				"sensorB": "HLX_DTC00",
				"trackPowerAPin":10,			
				"trackPowerBPin":11,
				"flipInterval":20
			}		
	)JSON";

	Document d;
	d.Parse(json);	

	ASSERT_THROW(TurntableAutoInverterDecoder( DccAddress { 128 }, dcclite::RName{"test"}, g_DecoderServices, g_DeviceDecoderServices, d ), std::invalid_argument);
}
