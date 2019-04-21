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

#include "Storage.h"

OutputDecoder::OutputDecoder(dcclite::Packet &packet) :
	Decoder::Decoder(packet)
{
	m_tPin = packet.Read<Pin_t>();
	m_fFlags = packet.Read<uint8_t>();	

	using namespace dcclite;
	
	this->Init();
}

OutputDecoder::OutputDecoder(EpromStream &stream):
	Decoder::Decoder(stream)
{	
	stream.Get(m_tPin);

	m_uStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);

	this->Init();
}


void OutputDecoder::SaveConfig(EpromStream &stream)
{
	Decoder::SaveConfig(stream);

	m_uStorageIndex = stream.GetIndex();

	stream.Put(m_tPin);
	stream.Put(m_fFlags);
}

void OutputDecoder::Init()
{
	using namespace dcclite;

	pinMode(m_tPin, OUTPUT);

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	m_fFlags |= ((m_fFlags & OUTD_IGNORE_SAVED_STATE) ? (m_fFlags & OUTD_ACTIVATE_ON_POWER_UP) : 0) ? OUTD_ACTIVE : 0;

	digitalWrite(m_tPin, (m_fFlags & OUTD_ACTIVE) ^ (m_fFlags & OUTD_INVERTED_OPERATION));
}

bool OutputDecoder::AcceptServerState(dcclite::DecoderStates state)
{
	using namespace dcclite;

	bool activate = state == dcclite::DecoderStates::ACTIVE;
	bool currentState = m_fFlags & OUTD_ACTIVE;

	//no state change?
	if (currentState == activate)
		return false;

	//Which state should we use?
	if (activate)
		m_fFlags |= OUTD_ACTIVE;
	else
		m_fFlags &= ~OUTD_ACTIVE;

	//Now set pin state
	digitalWrite(m_tPin, (m_fFlags & OUTD_ACTIVE) ^ (m_fFlags & OUTD_INVERTED_OPERATION));

	//Store current state on eprom, so we can reload.
	Storage::UpdateField(m_uStorageIndex, m_fFlags);

	return true;
}
