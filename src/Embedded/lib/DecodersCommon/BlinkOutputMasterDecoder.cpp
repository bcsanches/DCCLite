// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "BlinkOutputMasterDecoder.h"

#include "Console.h"
#include "DecoderManager.h"

#include "BlinkOutputDecoder.h"

#define MODULE_NAME F("BlinkOutputMaster")

BlinkOutputMasterDecoder::BlinkOutputMasterDecoder(dcclite::Packet &packet) :
	Decoder{ packet },
	m_fFlags{ packet },
	m_u8BlinkDecoderIndex{ packet.Read<uint8_t>() }
{
	//empty
}

BlinkOutputMasterDecoder::BlinkOutputMasterDecoder(Storage::EpromStream &stream) :
	Decoder{ stream },
	m_fFlags{ stream },
	m_u8BlinkDecoderIndex{ stream.Read<uint8_t>() }
{
	//empty
}

void BlinkOutputMasterDecoder::SaveConfig(Storage::EpromStream &stream)
{
	Decoder::SaveConfig(stream);
	m_fFlags.Save(stream);

	stream.Put(m_u8BlinkDecoderIndex);
}

bool BlinkOutputMasterDecoder::AcceptServerState(dcclite::DecoderStates state, const unsigned long time)
{
	bool active = state == dcclite::DecoderStates::ACTIVE;
	if (this->IsActive() == active)
		return false;

	m_fFlags.SetActiveBit(active);
	m_fFlags.PersistState();

	auto decoder = DecoderManager::TryGetDecoder(m_u8BlinkDecoderIndex);
	if (decoder == nullptr)
	{
		Console::Printf(F("[%z] %z %d\n"), MODULE_NAME, F("AcceptServerState"), ' ', F("no"), ' ', F("slave"), ':', ' ', m_u8BlinkDecoderIndex);

		return true;
	}

	if (decoder->GetType() != dcclite::DecoderTypes::DEC_BLINK_OUTPUT)
	{
		Console::Printf(F("[%z] %z %d\n"), MODULE_NAME, F("AcceptServerState"), ' ', F("slave"), F("is not blink output: "), m_u8BlinkDecoderIndex, ',', decoder->GetType());

		return true;
	}

	auto blinkDecoder = static_cast<BlinkOutputDecoder *>(decoder);
	
	blinkDecoder->SetBlinkingState(state);	

	return true;
}
