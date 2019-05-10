#include "SensorDecoder.h"

#include <Packet.h>

static Decoder::Class sensorDecoder("Sensor",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, const std::string &name, DccLiteService &owner, const rapidjson::Value &params)
	-> std::unique_ptr<Decoder> { return std::make_unique<SensorDecoder>(decoderClass, address, name, owner, params); }
);

SensorDecoder::SensorDecoder(const Class &decoderClass,
	const Address &address,
	const std::string &name,
	DccLiteService &owner,
	const rapidjson::Value &params
):
	Decoder(decoderClass, address, name, owner, params),
	m_iPin(params["pin"].GetInt())
{
	auto pullup = params.FindMember("pullup");
	m_fPullUp = pullup != params.MemberEnd() ? pullup->value.GetBool() : false;
}

void SensorDecoder::WriteConfig(dcclite::Packet &packet) const
{
	Decoder::WriteConfig(packet);

	packet.Write8(m_iPin);
	packet.Write8(
		(m_fPullUp ? dcclite::SensorDecoderFlags::SENSOR_PULL_UP : 0)
	);
}
