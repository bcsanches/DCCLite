#pragma once

#include "Decoder.h"

namespace dcclite
{
	class Packet;
}

class OutputDecoder : public Decoder
{	
	private:
		Pin_t	m_tPin = null_pin;
		uint8_t	m_fFlags = 0;

	public:
		OutputDecoder(dcclite::Packet &packet);
};
