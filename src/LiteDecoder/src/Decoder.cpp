#include "Decoder.h"

#include "Packet.h"

Decoder::Decoder(dcclite::Packet &packet)
{
	//dcc address, ignored
	packet.Read<uint16_t>();
}
