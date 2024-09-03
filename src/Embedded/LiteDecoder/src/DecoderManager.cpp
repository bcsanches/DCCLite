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
#include "QuadInverterDecoder.h"
#include "SensorDecoder.h"
#include "ServoTurnoutDecoder.h"
#include "Session.h"
#include "Storage.h"
#include "TurntableAutoInverterDecoder.h"

#include <Packet.h>

#if (defined ARDUINO_AVR_MEGA2560) || (defined DCCLITE_ARDUINO_EMULATOR)
#define MAX_DECODERS 48
#else
constexpr auto MAX_DECODERS = 16;
#endif

static Decoder *g_pDecoders[MAX_DECODERS] = { 0 };

#define MODULE_NAME					F("DecoderMgr")

#define FSTR_SLOT_IN_USE			F("Slot already in use")

#define FSTR_SLOT_OUT_OF_RANGE		F("Slot out of range")

#define FSTR_INVALID_DECODER_TYPE	F("Invalid decoder type")

static Decoder *Create(const dcclite::DecoderTypes type, dcclite::Packet &packet)
{
	switch (type)
	{
		case dcclite::DecoderTypes::DEC_OUTPUT:
			return new OutputDecoder(packet);

		case dcclite::DecoderTypes::DEC_SENSOR:
			return new SensorDecoder(packet);

		case dcclite::DecoderTypes::DEC_SERVO_TURNOUT:
			return new ServoTurnoutDecoder(packet);

		case dcclite::DecoderTypes::DEC_QUAD_INVERTER:
			return new QuadInverterDecoder(packet);

		case dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER:
			return new TurntableAutoInverterDecoder(packet);		

		default:
			//Console::SendLogEx(MODULE_NAME, FSTR_INVALID_DECODER_TYPE, static_cast<int>(type));
			DCCLITE_LOG_MODULE_LN(FSTR_INVALID_DECODER_TYPE << ' ' << static_cast<int>(type));

			return nullptr;
	}
}

Decoder *DecoderManager::Create(const uint8_t slot, dcclite::Packet &packet)
{
	if (slot >= MAX_DECODERS)
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_SLOT_OUT_OF_RANGE, ' ', slot);
		DCCLITE_LOG_MODULE_LN(FSTR_SLOT_OUT_OF_RANGE << ' ' << slot);		

		return nullptr;
	}

	if (g_pDecoders[slot])
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_SLOT_IN_USE, slot);
		DCCLITE_LOG_MODULE_LN(FSTR_SLOT_IN_USE << ' ' << slot);

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
		//Console::SendLogEx(MODULE_NAME, FSTR_SLOT_OUT_OF_RANGE, ' ', slot);
		DCCLITE_LOG << MODULE_NAME << FSTR_SLOT_OUT_OF_RANGE << ' ' << slot << DCCLITE_ENDL;

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

void DecoderManager::SaveConfig(Storage::EpromStream &stream)
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
		dcclite::DecoderTypes::DEC_SERVO_TURNOUT,
		dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER,
		dcclite::DecoderTypes::DEC_QUAD_INVERTER,

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

void DecoderManager::LoadConfig(Storage::EpromStream &stream)
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

	//Console::SendLogEx(MODULE_NAME, 'O', ' ', (int)sizeof(OutputDecoder), ' ', 'S', ' ', (int)sizeof(SensorDecoder), ' ', 'T', ' ', (int)sizeof(ServoTurnoutDecoder));
	DCCLITE_LOG << MODULE_NAME << F("O ") << (int)sizeof(OutputDecoder) << F(" S ") << (int)sizeof(SensorDecoder) << F(" T ") << (int)sizeof(ServoTurnoutDecoder) << DCCLITE_ENDL;

	uint16_t usedMem = 0;
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

					usedMem += sizeof(OutputDecoder);

					//Console::Send('O');
					DCCLITE_LOG << 'O';
					break;				

				case dcclite::DecoderTypes::DEC_SENSOR:
					decoder = new SensorDecoder(stream);

					usedMem += sizeof(SensorDecoder);

					//Console::Send('S');
					DCCLITE_LOG << 'S';
					break;

				case dcclite::DecoderTypes::DEC_SERVO_TURNOUT:
					decoder = new ServoTurnoutDecoder(stream);

					usedMem += sizeof(ServoTurnoutDecoder);

					//Console::Send('T');
					DCCLITE_LOG << 'T';
					break;

				case dcclite::DecoderTypes::DEC_TURNTABLE_AUTO_INVERTER:
					decoder = new TurntableAutoInverterDecoder(stream);

					usedMem += sizeof(TurntableAutoInverterDecoder);

					//Console::Send('U');
					DCCLITE_LOG << 'U';
					break;

				case dcclite::DecoderTypes::DEC_QUAD_INVERTER:
					decoder = new QuadInverterDecoder(stream);

					usedMem += sizeof(QuadInverterDecoder);

					DCCLITE_LOG << 'Q';
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
	
	//Console::SendLn("");
	//Console::SendLogEx(MODULE_NAME, usedMem);	
	DCCLITE_LOG << DCCLITE_ENDL;
	DCCLITE_LOG_MODULE_LN(usedMem);
}

bool DecoderManager::ReceiveServerStates(const dcclite::StatesBitPack_t &changedStates, const dcclite::StatesBitPack_t &states, const unsigned long time)
{
	bool stateChanged = false;
	for (unsigned i = 0; (i < changedStates.size()) && (i < MAX_DECODERS); ++i)
	{
		if (!changedStates[i])
			continue;

		//Console::SendLogEx(MODULE_NAME, "state", ' ', "for", i, "is",' ', states[i]);

		auto *decoder = g_pDecoders[i];
		if (!decoder)
			continue;

		decoder->AcceptServerState(states[i] ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE, time);

		if (decoder->IsOutputDecoder())
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

void DecoderManager::WriteOutputDecoderStates(dcclite::StatesBitPack_t &changedStates, dcclite::StatesBitPack_t &states)
{
	changedStates.ClearAll();
	states.ClearAll();	

	for (unsigned i = 0; i < MAX_DECODERS; ++i)
	{
		if (!g_pDecoders[i])
			continue;

		if (!g_pDecoders[i]->IsOutputDecoder())
			continue;

		changedStates.SetBit(i);
		states.SetBitValue(i, g_pDecoders[i]->IsActive());		
	}	
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

		stateChanged = g_pDecoders[i]->Update(ticks) || stateChanged;
	}

	return stateChanged;
}


Decoder *DecoderManager::TryPopDecoder(const uint8_t slot) noexcept
{
	if (slot >= MAX_DECODERS)
	{
		//Console::SendLogEx(MODULE_NAME, F("TryPopDecoder:"), FSTR_SLOT_OUT_OF_RANGE, ' ', slot);
		DCCLITE_LOG_MODULE_LN(F("TryPopDecoder:") << FSTR_SLOT_OUT_OF_RANGE << ' ' << slot);		

		return nullptr;
	}

	auto dec = g_pDecoders[slot];
	g_pDecoders[slot] = nullptr;

	return dec;
}

bool DecoderManager::PushDecoder(Decoder *decoder, const uint8_t slot) noexcept
{
	if (slot >= MAX_DECODERS)
	{
		//Console::SendLogEx(MODULE_NAME, F("PushDecoder:"), FSTR_SLOT_OUT_OF_RANGE, ' ', slot);		
		DCCLITE_LOG_MODULE_LN(F("PushDecoder:") << FSTR_SLOT_OUT_OF_RANGE << ' ' << slot);

		return false;
	}

	if (g_pDecoders[slot])
	{
		//Console::SendLogEx(MODULE_NAME, FSTR_SLOT_IN_USE, slot);		
		DCCLITE_LOG_MODULE_LN(F("PushDecoder:") << FSTR_SLOT_IN_USE << ' ' << slot);

		return false;
	}

	g_pDecoders[slot] = decoder;

	return true;
}

Decoder *DecoderManager::TryGetDecoder(const uint8_t slot) noexcept
{
	if (slot >= MAX_DECODERS)
	{
		//Console::SendLogEx(MODULE_NAME, F("TryGetDecoder:"), FSTR_SLOT_OUT_OF_RANGE, ' ', slot);
		DCCLITE_LOG_MODULE_LN(F("TryGetDecoder:") << FSTR_SLOT_OUT_OF_RANGE << ' ' << slot);		

		return nullptr;
	}

	return g_pDecoders[slot];
}

bool DecoderManager::GetDecoderActiveStatus(const uint8_t slot, bool &result) noexcept
{
	if (slot >= MAX_DECODERS)
	{
		//Console::SendLogEx(MODULE_NAME, F("IsDecoderActive:"), FSTR_SLOT_OUT_OF_RANGE, ' ', slot);
		DCCLITE_LOG_MODULE_LN(F("IsDecoderActive:") << FSTR_SLOT_OUT_OF_RANGE << ' ' << slot);

		return false;
	}

	if (!g_pDecoders[slot])
	{
		//Console::SendLogEx(MODULE_NAME, F("IsDecoderActive:"), F(" slot is empty"), ' ', slot);
		DCCLITE_LOG_MODULE_LN(F("IsDecoderActive: slot is empty") << ' ' << slot);		

		return false;
	}

	result = g_pDecoders[slot]->IsActive();

	return true;
}


