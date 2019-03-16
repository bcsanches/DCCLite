#include "OutputDecoder.h"

#include "Packet.h"

static Decoder::Class outputDecoder("Output",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, const std::string &name, DccLiteService &owner, const rapidjson::Value &params)
		-> std::unique_ptr<Decoder> { return std::make_unique<OutputDecoder>(decoderClass, address, name, owner, params); }
);


OutputDecoder::OutputDecoder(
	const Class &decoderClass,
	const Address &address,
	const std::string &name,
	DccLiteService &owner,
	const rapidjson::Value &params
) :
	Decoder(decoderClass, address, name, owner, params),
	m_iPin(params["pin"].GetInt())
{
	auto inverted = params.FindMember("inverted");	
	m_fInvertedOperation = inverted != params.MemberEnd() ? inverted->value.GetBool() : false;

	auto setOnPower = params.FindMember("ignoreSavedState");
	m_fIgnoreSavedState = setOnPower != params.MemberEnd() ? setOnPower->value.GetBool() : false;

	auto activateOnPowerUp = params.FindMember("activateOnPowerUp");
	m_fActivateOnPowerUp = activateOnPowerUp != params.MemberEnd() ? activateOnPowerUp->value.GetBool() : false;
}


void OutputDecoder::WriteConfig(dcclite::Packet &packet) const
{
	Decoder::WriteConfig(packet);

	packet.Write8(m_iPin);
	packet.Write8(
		(m_fInvertedOperation ? dcclite::OutputDecoderFlags::OUTD_INVERTED_OPERATION : 0) |
		(m_fIgnoreSavedState ? dcclite::OutputDecoderFlags::OUTD_IGNORE_SAVED_STATE : 0) |
		(m_fActivateOnPowerUp ? dcclite::OUTD_ACTIVATE_ON_POWER_UP : 0)
	);
}
