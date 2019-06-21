// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ServoTurnoutDecoder.h"

#include "Console.h"
#include "Packet.h"
#include "Storage.h"

ServoTurnoutDecoder::ServoTurnoutDecoder(dcclite::Packet& packet) :
	Decoder::Decoder(packet)
{
	m_tPin = packet.Read<Pin_t>();

	m_fFlags = packet.Read<uint8_t>();

	this->Init();
}

ServoTurnoutDecoder::ServoTurnoutDecoder(EpromStream& stream) :
	Decoder::Decoder(stream)
{	
	stream.Get(m_tPin);

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);

	this->Init();
}

ServoTurnoutDecoder::~ServoTurnoutDecoder()
{
	m_clServo.detach();
}


void ServoTurnoutDecoder::SaveConfig(EpromStream& stream)
{
	Decoder::SaveConfig(stream);

	stream.Put(m_tPin);

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Put(m_fFlags);
}

void ServoTurnoutDecoder::OperatePin()
{
	using namespace dcclite;	

	bool active = (m_fFlags & SRVT_ACTIVE);
	active = (m_fFlags & SRVT_INVERTED_OPERATION) ? !active : active;

	m_clServo.write(active ? 20 : 0);	
	//m_clServo.writeMicroseconds(active ? 0 : 1300); 
	//Console::SendLogEx("ServoTurnoutDecoder", "Operate Pin", active ? 90 : 0);

	//Store current state on eprom, so we can reload.
	if (m_uFlagsStorageIndex)
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);

#if 1
	Console::SendLogEx("[ServoTurnout]", ' ', "PIN: ", m_tPin);
	Console::SendLogEx("[ServoTurnout]", ' ', "IGNORE_SAVE: ", m_fFlags & SRVT_IGNORE_SAVED_STATE);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVATE_ON_POWERUP: ", m_fFlags & SRVT_ACTIVATE_ON_POWER_UP);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVE: ", m_fFlags & SRVT_ACTIVE);
	Console::SendLogEx("[ServoTurnout]", ' ', "INVERTED: ", m_fFlags & SRVT_INVERTED_OPERATION);
#endif
}

void ServoTurnoutDecoder::Init()
{
	using namespace dcclite;
	
	m_clServo.attach(m_tPin);

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	if (m_fFlags & SRVT_IGNORE_SAVED_STATE)
	{
		if (m_fFlags & SRVT_ACTIVATE_ON_POWER_UP)
			m_fFlags |= SRVT_ACTIVE;
		else
			m_fFlags &= ~SRVT_ACTIVE;
	}

#if 1
	Console::SendLogEx("[ServoTurnout]", ' ', "PIN: ", m_tPin);
	Console::SendLogEx("[ServoTurnout]", ' ', "IGNORE_SAVE: ", m_fFlags & SRVT_IGNORE_SAVED_STATE);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVATE_ON_POWERUP: ", m_fFlags & SRVT_ACTIVATE_ON_POWER_UP);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVE: ", m_fFlags & SRVT_ACTIVE);
	Console::SendLogEx("[ServoTurnout]", ' ', "INVERTED: ", m_fFlags & SRVT_INVERTED_OPERATION);
#endif

	this->OperatePin();
}

bool ServoTurnoutDecoder::AcceptServerState(dcclite::DecoderStates state)
{
	using namespace dcclite;

	bool activate = state == dcclite::DecoderStates::ACTIVE;
	bool currentState = m_fFlags & SRVT_ACTIVE;

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
		m_fFlags |= SRVT_ACTIVE;
	else
		m_fFlags &= ~SRVT_ACTIVE;

#if 0
	Console::SendLogEx("[OutputDecoder]", ' ', "FLAGS: ", m_fFlags);
	Console::SendLogEx("[OutputDecoder]", ' ', "ACTIVE: ", m_fFlags & OUTD_ACTIVE);
#endif

	//Now set pin state
	this->OperatePin();

	return true;
}


