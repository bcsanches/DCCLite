#pragma once

#include <ostream>

#include "ClassInfo.h"

#include "json.hpp"

class DecoderManager;
class Node;

class Decoder
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

			private:
				int m_iAddress;

				friend std::ostream& operator<<(std::ostream& os, const Address& address);
		};

		typedef dcclite::ClassInfo<Decoder, const Address &, const std::string &, DecoderManager &, const nlohmann::json &> Class;

	public:
		Decoder(
			const Class &decoderClass, 
			const Address &address, 
			const std::string &name,
			DecoderManager &owner, 
			const nlohmann::json &params
		);

	private:
		Address m_iAddress;
		std::string m_strName;

		DecoderManager &m_rclManager;
};

inline std::ostream &operator<<(std::ostream& os, const Decoder::Address &address)
{
	os << address.m_iAddress;

	return os;
}


