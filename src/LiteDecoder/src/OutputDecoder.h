#pragma once

#include "Decoder.h"

class OutputDecoder : public Decoder
{	
	private:
		uint16_t	m_uStorageIndex = 0;
		Pin_t		m_tPin = null_pin;
		uint8_t		m_fFlags = 0;

	public:
		OutputDecoder(dcclite::Packet &packet);
		OutputDecoder(EpromStream &stream);

		virtual void SaveConfig(EpromStream &stream);

		virtual dcclite::DecoderTypes GetType() const 
		{
			return dcclite::DecoderTypes::DEC_OUTPUT;
		};

	private:
		void Init();
};
