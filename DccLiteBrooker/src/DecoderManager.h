#pragma once

#include <map>

#include "Decoder.h"

class DecoderManager
{
	public:
		DecoderManager() = default;

	private:
		std::map<Decoder::Address, Decoder> m_mapDecoders;
};

