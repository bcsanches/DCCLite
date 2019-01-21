#pragma once

#include "Decoder.h"

class OutputDecoder : public Decoder
{
	private:
		Pin_t	m_tPin = null_pin;
		bool	m_fSet = false;

	public:
		OutputDecoder() :
			Decoder(dcclite::DecoderTypes::DEC_OUTPUT)			
		{
			//empty
		}
};
