#include "MiniDuino.h"

#include <assert.h>

#include <Arduino.h>
#include <ArduinoLib.h>

#include <Decoder.h>

MiniDuino *MiniDuino::g_pclSingleton = nullptr;

MiniDuino::MiniDuino()
{
	assert(!g_pclSingleton);

	g_pclSingleton = this;

	ArduinoLib::Setup(Setup, Loop, "UnitTestMiniDuino");
}

MiniDuino::~MiniDuino()
{
	ArduinoLib::Finalize();

	g_pclSingleton = nullptr;
}

void MiniDuino::AddDecoder(Decoder &dec)
{
	m_vecDecoders.push_back(&dec);
}

void MiniDuino::Loop()
{
	auto ticks = millis();

	for (auto dec : g_pclSingleton->m_vecDecoders)
	{
		dec->Update(ticks);
	}
}
