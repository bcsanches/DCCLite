#include "Decoder.h"

Decoder::Decoder(const Class &decoderClass, const Address &address, const std::string &name, DecoderManager &owner, const nlohmann::json &params):
	m_iAddress(address),
	m_strName(name),
	m_rclManager(owner)
{
	//empty
}
