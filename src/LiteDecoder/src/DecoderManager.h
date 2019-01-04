#pragma once

#include "EmbeddedLibDefs.h"

class Decoder;

namespace DecoderManager
{
	Decoder *Create(uint8_t slot, dcclite::DecoderTypes type);
	void Destroy(uint8_t slot);
}
