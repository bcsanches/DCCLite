#include "Decoder.h"

#include "Packet.h"
#include "Parser.h"

Decoder::Address::Address(const rapidjson::Value &value)
{
	if (value.IsString())
	{		
		dcclite::Parser parser{ value.GetString() };
		
		int adr;		
		if (parser.GetNumber(adr) != dcclite::TOKEN_NUMBER)
		{					
			throw std::runtime_error(fmt::format("error: Decoder::Address::Address(const nlohmann::json::value_type &value) invalid value for address, see {}", value.GetString()));
		}		

		m_iAddress = adr;
	}
	else
	{
		m_iAddress = value.GetInt();
	}
}

Decoder::Decoder(const Class &decoderClass, const Address &address, std::string name, DccLiteService &owner, const rapidjson::Value &params):
	Object(std::move(name)),
	m_iAddress(address),	
	m_rclManager(owner)
{
	//empty
}


void Decoder::WriteConfig(dcclite::Packet &packet) const
{
	packet.Write8(static_cast<std::uint8_t>(this->GetType()));
	m_iAddress.WriteConfig(packet);
}

void Decoder::Address::WriteConfig(dcclite::Packet &packet) const
{
	packet.Write16(m_iAddress);
}
