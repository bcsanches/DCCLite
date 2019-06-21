// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "OutputDecoder.h"

#include <Arduino.h>
#include <EmbeddedLibDefs.h>
#include <Packet.h>

#include "Console.h"
#include "Storage.h"

OutputDecoder::OutputDecoder(dcclite::Packet &packet) :
	Decoder::Decoder{ packet },
	m_clPin{ packet.Read<Pin_t>(), dcclite::Pin::MODE_OUTPUT }
{	
	m_fFlags = packet.Read<uint8_t>();	

	using namespace dcclite;
	
	this->Init();
}

OutputDecoder::OutputDecoder(EpromStream &stream):
	Decoder::Decoder(stream)
{	
	unsigned char pin;
	stream.Get(pin);

	m_clPin.Attach(pin, dcclite::Pin::MODE_OUTPUT);

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);

	this->Init();
}


void OutputDecoder::SaveConfig(EpromStream &stream)
{
	Decoder::SaveConfig(stream);	

	stream.Put(m_clPin.Num());

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Put(m_fFlags);
}

void OutputDecoder::OperatePin()
{
	using namespace dcclite;	

	bool active = (m_fFlags & OUTD_ACTIVE);	
	active = (m_fFlags & OUTD_INVERTED_OPERATION) ? !active : active;	

	m_clPin.DigitalWrite(active);	

	//Store current state on eprom, so we can reload.
	if(m_uFlagsStorageIndex)
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);
}

void OutputDecoder::Init()
{
	using namespace dcclite;
	
	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	if(m_fFlags & OUTD_IGNORE_SAVED_STATE)
	{
		if(m_fFlags & OUTD_ACTIVATE_ON_POWER_UP)
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

	this->OperatePin();
}

bool OutputDecoder::AcceptServerState(dcclite::DecoderStates state)
{
	using namespace dcclite;

	bool activate = state == dcclite::DecoderStates::ACTIVE;
	bool currentState = m_fFlags & OUTD_ACTIVE;	

	//no state change?
	if (currentState == activate)
	{
#if 0
		Console::SendLogEx("[OutputDecoder]", "got state, but ignored (same)");
#endif

		return false;
	}

	//Which state should we use?
	if (activate)
		m_fFlags |= OUTD_ACTIVE;
	else	
		m_fFlags &= ~OUTD_ACTIVE;

#if 0
	Console::SendLogEx("[OutputDecoder]", ' ', "FLAGS: ", m_fFlags);
	Console::SendLogEx("[OutputDecoder]", ' ', "ACTIVE: ", m_fFlags & OUTD_ACTIVE);	
#endif

	//Now set pin state
	this->OperatePin();	

	return true;
}
