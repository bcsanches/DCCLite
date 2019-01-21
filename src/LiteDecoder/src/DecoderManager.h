#pragma once

#include "EmbeddedLibDefs.h"

class Decoder;

namespace dcclite
{
	class Packet;
}

namespace DecoderManager
{
	Decoder *Create(const uint8_t slot, dcclite::Packet &packet);

	void Destroy(const uint8_t slot);
}
