#pragma once

#include "Decoder.h"

class OutputDecoder : public Decoder
{
	public:
		OutputDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			DecoderManager &owner,
			const nlohmann::json &params
		);

	private:
		int m_iPin;
};