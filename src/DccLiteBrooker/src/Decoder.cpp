#include "Decoder.h"

#include "Parser.h"

Decoder::Address::Address(const nlohmann::json::value_type &value)
{
	if (value.is_string())
	{
		auto str = value.get<std::string>();

		dcclite::Parser parser{ str.c_str() };
		
		auto token = parser.GetNumber(m_iAddress);
		if (token != dcclite::TOKEN_NUMBER)
		{
			std::stringstream stream;

			stream << "error: Decoder::Address::Address(const nlohmann::json::value_type &value) invalid value for address, see " << value;

			throw std::runtime_error(stream.str());
		}		
	}
	else
	{
		m_iAddress = value.get<int>();
	}
}

Decoder::Decoder(const Class &decoderClass, const Address &address, const std::string &name, DecoderManager &owner, const nlohmann::json &params):
	m_iAddress(address),
	m_strName(name),
	m_rclManager(owner)
{
	//empty
}
