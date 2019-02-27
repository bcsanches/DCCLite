#pragma once

#include <ostream>

#include "ClassInfo.h"
#include "EmbeddedLibDefs.h"
#include "Object.h"

#include "json.hpp"

#include <fmt/format.h>

class DccLiteService;
class Node;

namespace dcclite
{
	class Packet;
}

class Decoder: public dcclite::Object
{
	public:
		class Address
		{
			public:
				inline Address(int address) :
					m_iAddress(address)
				{
					//empty
				}

				Address(const nlohmann::json::value_type &value);

				Address() = default;
				Address(const Address &) = default;
				Address(Address &&) = default;		

				inline int GetAddress() const
				{
					return m_iAddress;
				}

				inline bool operator<(const Address &rhs) const
				{
					return m_iAddress < rhs.m_iAddress;
				}

				std::string ToString() const
				{
					return fmt::format("{:#05x}", m_iAddress);					
				}

				void WriteConfig(dcclite::Packet &packet) const;

			private:
				int16_t m_iAddress;

				friend std::ostream& operator<<(std::ostream& os, const Address& address);
		};

		typedef dcclite::ClassInfo<Decoder, const Address &, const std::string &, DccLiteService &, const nlohmann::json &> Class;

	public:
		Decoder(
			const Class &decoderClass, 
			const Address &address, 
			std::string name,
			DccLiteService &owner,
			const nlohmann::json &params
		);

		inline Address GetAddress() const
		{
			return m_iAddress;
		}

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept = 0;

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "Decoder";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Object::Serialize(stream);

			stream.AddIntValue("address", m_iAddress.GetAddress());
		}

	private:
		Address m_iAddress;		

		DccLiteService &m_rclManager;
};

inline std::ostream &operator<<(std::ostream& os, const Decoder::Address &address)
{
	os << address.m_iAddress;

	return os;
}

