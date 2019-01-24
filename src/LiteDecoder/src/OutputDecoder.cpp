#include "OutputDecoder.h"

#include <Packet.h>

OutputDecoder::OutputDecoder(dcclite::Packet &packet):
	Decoder(dcclite::DecoderTypes::DEC_OUTPUT)
{
	m_tPin = packet.Read<Pin_t>();
	m_fFlags = packet.Read<uint8_t>();


}