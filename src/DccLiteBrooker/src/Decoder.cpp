#include "Decoder.h"

#include "Packet.h"
#include "Parser.h"

Decoder::Address::Address(const nlohmann::json::value_type &value)
{
	if (value.is_string())
	{
		auto str = value.get<std::string>();

		dcclite::Parser parser{ str.c_str() };
		
		int adr;		
		if (parser.GetNumber(adr) != dcclite::TOKEN_NUMBER)
		{
			std::stringstream stream;

			stream << "error: Decoder::Address::Address(const nlohmann::json::value_type &value) invalid value for address, see " << value;

			throw std::runtime_error(stream.str());
		}		

		m_iAddress = adr;
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


void Decoder::WriteConfig(dcclite::Packet &packet)
{
	packet.Write8(static_cast<std::uint8_t>(this->GetType()));
	m_iAddress.WriteConfig(packet);
}

void Decoder::Address::WriteConfig(dcclite::Packet &packet)
{
	packet.Write16(m_iAddress);
}
