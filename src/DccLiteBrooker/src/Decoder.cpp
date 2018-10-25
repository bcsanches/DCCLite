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

Decoder::Decoder(const Class &decoderClass, const Address &address, std::string name, DccLiteService &owner, const nlohmann::json &params):
	Object(std::move(name)),
	m_iAddress(address),	
	m_rclManager(owner)
{
	//empty
}
