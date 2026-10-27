#pragma once

#include <vector>

class Decoder;

class MiniDuino
{
	private:
		static void Setup()
		{

		}

		static void Loop();

	public:
		MiniDuino();

		~MiniDuino();

		void AddDecoder(Decoder &dec);

	private:
		std::vector<Decoder *> m_vecDecoders;

		static MiniDuino *g_pclSingleton;
};