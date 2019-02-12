#pragma once

#include "EmbeddedLibDefs.h"

typedef unsigned char Pin_t;

constexpr Pin_t null_pin = 255;

class EpromStream;

class Decoder
{	
	public:
		Decoder()			
		{
			//empty
		}

		virtual dcclite::DecoderTypes GetType() const = 0;

		virtual ~Decoder() = default;

		virtual void SaveConfig(EpromStream &stream)
		{
			//empty
		}

		Decoder(const Decoder &) = delete;
		Decoder(const Decoder &&) = delete;
		Decoder &operator=(const Decoder &) = delete;
};
