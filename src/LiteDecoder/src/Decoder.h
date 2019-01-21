#pragma once

#include "EmbeddedLibDefs.h"

typedef unsigned char Pin_t;

constexpr Pin_t null_pin = 255;

class Decoder
{
	private:
		const dcclite::DecoderTypes mType;

	public:
		Decoder(const dcclite::DecoderTypes type) :
			mType{ type }
		{
			//empty
		}

		virtual ~Decoder() = default;

		Decoder(const Decoder &) = delete;
		Decoder(const Decoder &&) = delete;
		Decoder operator=(const Decoder &) = delete;
};
