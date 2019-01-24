#include "DecoderManager.h"

#include "OutputDecoder.h"

#include "Console.h"

#include <Packet.h>

#define MAX_DECODERS 32

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

static const char DeviceManagerModuleName[] PROGMEM = { "DevMgr" };
#define MODULE_NAME Console::FlashStr(DeviceManagerModuleName)

static const char FStrSlotInUse[] PROGMEM = { "Slot already in use" };
#define FSTR_SLOT_IN_USE Console::FlashStr(FStrSlotInUse)

static const char FStrSlotOutOfRange[] PROGMEM = { "Slot out of range" };
#define FSTR_SLOT_OUT_OF_RANGE Console::FlashStr(FStrSlotOutOfRange)

static Decoder *Create(const dcclite::DecoderTypes type, dcclite::Packet &packet)
{
	switch (type)
	{
		case dcclite::DecoderTypes::DEC_OUTPUT:
			return new OutputDecoder(packet);

		case dcclite::DecoderTypes::DEC_INPUT:
		default:
			Console::SendLogEx(MODULE_NAME, "invalid", ' ', "dec", ' ', "type", ' ', static_cast<int>(type));

			return nullptr;
	}
}

Decoder *DecoderManager::Create(const uint8_t slot, dcclite::Packet &packet)
{
	if (slot >= MAX_DECODERS)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_SLOT_OUT_OF_RANGE, ' ', slot);

		return nullptr;
	}

	if (g_pDecoders[slot])
	{
		Console::SendLogEx(MODULE_NAME, FSTR_SLOT_IN_USE);
		return nullptr;
	}		

	auto decType = static_cast<dcclite::DecoderTypes>(packet.Read <uint8_t>());	

	auto decoder = ::Create(decType, packet);

	g_pDecoders[slot] = decoder;

	return decoder;
}

void DecoderManager::Destroy(const uint8_t slot)
{
	if (slot >= MAX_DECODERS)
	{
		Console::SendLogEx(MODULE_NAME, FSTR_SLOT_OUT_OF_RANGE, ' ', slot);

		return;
	}

	delete g_pDecoders[slot];
	g_pDecoders[slot] = nullptr;
}
