#include "DecoderManager.h"

#define MAX_DECODERS 32

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

Decoder *DecoderManager::Create(uint8_t slot, dcclite::DecoderTypes type)
{
	if (g_pDecoders[slot])
		return nullptr;

	return nullptr;
}
