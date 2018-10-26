#include "OutputDecoder.h"

static Decoder::Class outputDecoder("Output",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, const std::string &name, DccLiteService &owner, const nlohmann::json &params)
		-> std::unique_ptr<Decoder> { return std::make_unique<OutputDecoder>(decoderClass, address, name, owner, params); }
);


OutputDecoder::OutputDecoder(
	const Class &decoderClass,
	const Address &address,
	const std::string &name,
	DccLiteService &owner,
	const nlohmann::json &params
) :
	Decoder(decoderClass, address, name, owner, params),
	m_iPin(params["pin"].get<int>())
{
	//empty
}
