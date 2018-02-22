#pragma once

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

	public:
		Decoder(const Address &address, DecoderManager &owner);

	private:
		Address m_iAddress;

		DecoderManager &m_rclManager;
};
