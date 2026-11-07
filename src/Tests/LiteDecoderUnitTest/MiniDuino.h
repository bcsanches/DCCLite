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

		virtual ~MiniDuino();

		void AddDecoder(Decoder &dec);
		Decoder *GetDecoder(uint8_t index);

	
	protected:
		std::vector<Decoder *> m_vecDecoders;
};

class ManagedMiniDuino: public MiniDuino
{
	public:
		virtual ~ManagedMiniDuino();

		void Load(const char *json);

		void Store();
		void LoadFromStorage();
};
