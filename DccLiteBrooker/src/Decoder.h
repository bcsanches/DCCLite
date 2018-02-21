#pragma once

class DecoderManager;
class Node;

class Decoder
{
	public:
		Decoder(int address, DecoderManager &owner);

	private:
		int m_iAddress;

		DecoderManager &m_rclManager;
};
