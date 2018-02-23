#include "OutputDecoder.h"

static Decoder::Class outputDecoder("Output",
	[](const Decoder::Class &decoderClass, const Decoder::Address &address, DecoderManager &owner, const nlohmann::json &params) 
		-> std::unique_ptr<Decoder> { return std::make_unique<OutputDecoder>(decoderClass, address, owner, params); }
);


OutputDecoder::OutputDecoder(const Class &decoderClass, const Address &address, DecoderManager &owner, const nlohmann::json &params):
	Decoder(decoderClass, address, owner, params)
{
	//empty
}
