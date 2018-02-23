#include "Decoder.h"

Decoder::Decoder(const Class &decoderClass, const Address &address, DecoderManager &owner, const nlohmann::json &params):
	m_iAddress(address),
	m_rclManager(owner)
{
	//empty
}