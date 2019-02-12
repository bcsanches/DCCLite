#include "DecoderManager.h"

#include "Console.h"
#include "OutputDecoder.h"
#include "Storage.h"

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

void DecoderManager::SaveConfig(EpromStream &stream)
{
	dcclite::DecoderTypes types[] = { 
		dcclite::DecoderTypes::DEC_OUTPUT , 
		dcclite::DecoderTypes::DEC_INPUT,

		dcclite::DecoderTypes::DEC_NULL
	};

	//
	//First we save the decoder type, then we save all decoders of this kind:
	//[DecoderType]
	//[NumOfKind]
	//for each decoder of this type:
	//	[slot0]
	//	[decoderData0]
	//	...
	//	[slotn]
	//	[decoderDatan]

	//If there are no decoders of type, nothing is written for the type	
	//

	//This is quite slow, but keep memory usage low (at it is rare here)
	//For each type, run across all decoders and count how many are there
	//If more than zero, write type down, the number and then write each decoder
	for (int typeIndex = 0; types[typeIndex] != dcclite::DecoderTypes::DEC_NULL; ++typeIndex)
	{
		//We may mark the stream position and come back later to save the num, but in this case
		//we will have to put an if on the loop to mark the first position (to avoid saving number 0)
		//this will overcomplicate code, so lets just run throught the array twice
		uint8_t typeCount = 0;
		for (int i = 0; i < MAX_DECODERS; ++i)
		{
			if (g_pDecoders[i] && g_pDecoders[i]->GetType() == types[typeIndex])
				++typeCount;
		}

		if (!typeCount)
			continue;

		stream.Put(static_cast<uint8_t>(types[typeIndex]));
		stream.Put(typeCount);

		//Now that we counted, run across all decoders and save them
		for (int i = 0; i < MAX_DECODERS; ++i)
		{
			if (g_pDecoders[i] && g_pDecoders[i]->GetType() == types[typeIndex])
			{
				stream.Put(static_cast<uint8_t>(i));
				
				g_pDecoders[i]->SaveConfig(stream);
			}
		}
	}

	//write null type marker so we know we are done
	stream.Put(static_cast<uint8_t>(dcclite::DecoderTypes::DEC_NULL));
}
