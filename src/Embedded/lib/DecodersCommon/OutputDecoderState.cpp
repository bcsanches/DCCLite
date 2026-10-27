// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "OutputDecoderState.h"

#include <dcclite_shared/Packet.h>

#include "Storage.h"

OutputDecoderState::OutputDecoderState(dcclite::Packet &packet) :	
	m_clPin{ packet.Read<dcclite::PinType_t>(), Pin::MODE_OUTPUT }
{
	m_fFlags = packet.Read<uint8_t>();
	
	this->Init();
}

OutputDecoderState::OutputDecoderState(Storage::EpromStream &stream)
{
	dcclite::PinType_t pin;
	stream.Get(pin);

	m_clPin.Attach(pin, Pin::MODE_OUTPUT);

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);

	this->Init();
}

void OutputDecoderState::Init()
{
	using namespace dcclite;

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	if (m_fFlags & OUTD_IGNORE_SAVED_STATE)
	{
		if (m_fFlags & OUTD_ACTIVATE_ON_POWER_UP)
			m_fFlags |= OUTD_ACTIVE;
		else
			m_fFlags &= ~OUTD_ACTIVE;
	}

#if 0
	Console::SendLogEx("[OutputDecoder]", ' ', "PIN: ", m_tPin);
	Console::SendLogEx("[OutputDecoder]", ' ', "IGNORE_SAVE: ", m_fFlags & OUTD_IGNORE_SAVED_STATE);
	Console::SendLogEx("[OutputDecoder]", ' ', "ACTIVATE_ON_POWERUP: ", m_fFlags & OUTD_ACTIVATE_ON_POWER_UP);
	Console::SendLogEx("[OutputDecoder]", ' ', "ACTIVE: ", m_fFlags & OUTD_ACTIVE);
#endif
}



