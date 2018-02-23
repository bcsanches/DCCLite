#pragma once

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
				Address(int address) :
					m_iAddress(address)
				{
					//empty
				}

				Address() = default;
				Address(const Address &) = default;
				Address(Address &&) = default;				

			private:
				int m_iAddress;
		};

		typedef dcclite::ClassInfo<Decoder, const Address &, DecoderManager &, const nlohmann::json &> Class;

	public:
		Decoder(const Class &decoderClass, const Address &address, DecoderManager &owner, const nlohmann::json &params);

	private:
		Address m_iAddress;

		DecoderManager &m_rclManager;
};
