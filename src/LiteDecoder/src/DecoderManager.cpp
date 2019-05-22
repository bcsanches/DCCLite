// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DecoderManager.h"

#include "Console.h"
#include "OutputDecoder.h"
#include "SensorDecoder.h"
#include "Session.h"
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

static const char FStrInvalidDecoderType[] PROGMEM = { "Invalid decoder type" };
#define FSTR_INVALID_DECODER_TYPE Console::FlashStr(FStrSlotOutOfRange)

static Decoder *Create(const dcclite::DecoderTypes type, dcclite::Packet &packet)
{
	switch (type)
	{
		case dcclite::DecoderTypes::DEC_OUTPUT:
			return new OutputDecoder(packet);

		case dcclite::DecoderTypes::DEC_SENSOR:
			return new SensorDecoder(packet);
			break;

		default:
			Console::SendLogEx(MODULE_NAME, FSTR_INVALID_DECODER_TYPE, static_cast<int>(type));

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
		Console::SendLogEx(MODULE_NAME, FSTR_SLOT_IN_USE, slot);
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

void DecoderManager::DestroyAll()
{
	for (int i = 0; i < MAX_DECODERS; ++i)
	{
		delete g_pDecoders[i];
		g_pDecoders[i] = nullptr;
	}
}

void DecoderManager::SaveConfig(EpromStream &stream)
{	
	const dcclite::Guid &token = Session::GetConfigToken();

#ifdef WIN32
	static_assert(sizeof(dcclite::Guid::m_bId) == 16);
#endif

	for (unsigned int i = 0; i < sizeof(token.m_bId); ++i)
		stream.Put(token.m_bId[i]);

	dcclite::DecoderTypes types[] = { 
		dcclite::DecoderTypes::DEC_OUTPUT , 
		dcclite::DecoderTypes::DEC_SENSOR,

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

void DecoderManager::LoadConfig(EpromStream &stream)
{
	DestroyAll();

	dcclite::Guid configToken;

#ifdef WIN32
	static_assert(sizeof(dcclite::Guid::m_bId) == 16);
#endif

	for (unsigned int i = 0; i < sizeof(configToken.m_bId); ++i)
	{
		uint8_t byte;

		stream.Get(byte);

		configToken.m_bId[i] = byte;
	}

	for (;;)
	{
		uint8_t byte;

		stream.Get(byte);

		dcclite::DecoderTypes type = static_cast<dcclite::DecoderTypes>(byte);

		if (type == dcclite::DecoderTypes::DEC_NULL)
			break;

		stream.Get(byte);

		for (int i = 0; i < byte; ++i)
		{
			uint8_t slot;
			stream.Get(slot);

			//should we check or trust?
			//if(slot >= MAX_DECODERS)

			//should we check?
			//if(g_pDecoder[slot])

			//Console::SendLogEx(MODULE_NAME, "OUTD: ", i);

			Decoder *decoder = nullptr;

			switch (type)
			{
				case dcclite::DecoderTypes::DEC_OUTPUT:
					decoder = new OutputDecoder(stream);
					break;

				case dcclite::DecoderTypes::DEC_SENSOR:
					decoder = new SensorDecoder(stream);
					break;

				default:
					//should we check?
					break;
			}

			g_pDecoders[slot] = decoder;
		}
	}

	//loaded all decoders, set config token
	Session::ReplaceConfigToken(configToken);
}

bool DecoderManager::ReceiveServerStates(const dcclite::StatesBitPack_t &changedStates, const dcclite::StatesBitPack_t &states)
{
	bool stateChanged = false;
	for (unsigned i = 0; (i < changedStates.size()) && (i < MAX_DECODERS); ++i)
	{
		if (!changedStates[i])
			continue;

		//Console::SendLogEx(MODULE_NAME, "state", ' ', "for", i, "is",' ', states[i]);

		auto *decoder = g_pDecoders[i];
		decoder->AcceptServerState(states[i] ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);

		if (decoder->GetType() == dcclite::DecoderTypes::DEC_OUTPUT)
			stateChanged = true;
	}

	return stateChanged;
}

bool DecoderManager::ProduceStatesDelta(dcclite::StatesBitPack_t &changedStates, dcclite::StatesBitPack_t &states)
{
	changedStates.ClearAll();
	states.ClearAll();

	bool hasDelta = false;

	for (unsigned i = 0; i < MAX_DECODERS; ++i)
	{
		if (!g_pDecoders[i])
			continue;

		if (!g_pDecoders[i]->IsSyncRequired())
			continue;

		changedStates.SetBit(i);
		states.SetBitValue(i, g_pDecoders[i]->IsActive());

		hasDelta = true;
	}

	return hasDelta;
}

void DecoderManager::WriteStates(dcclite::StatesBitPack_t &changedStates, dcclite::StatesBitPack_t &states)
{
	changedStates.ClearAll();
	states.ClearAll();

	for (unsigned i = 0; i < MAX_DECODERS; ++i)
	{
		if (!g_pDecoders[i])
			continue;

		changedStates.SetBit(i);
		states.SetBitValue(i, g_pDecoders[i]->IsActive());
	}
}

bool DecoderManager::Update(const unsigned long ticks)
{
	bool stateChanged = false;

	for (size_t i = 0; i < MAX_DECODERS; ++i)
	{
		if (!g_pDecoders[i])
			continue;

		stateChanged |= g_pDecoders[i]->Update(ticks);
	}

	return stateChanged;
}

