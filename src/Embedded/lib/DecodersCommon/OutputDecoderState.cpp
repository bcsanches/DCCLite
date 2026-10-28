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

void OutputDecoderState::SaveConfig(Storage::EpromStream &stream)
{
	stream.Put(m_clPin.Raw());

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Put(m_fFlags);
}

bool OutputDecoderState::SetActive(dcclite::DecoderStates state)
{
	bool active = state == dcclite::DecoderStates::ACTIVE;
	if (this->IsActive() == active)
		return false;

	if (active)
		m_fFlags |= dcclite::OUTD_ACTIVE;
	else
		m_fFlags &= ~dcclite::OUTD_ACTIVE;

	this->SetOutput(state);
	this->PersistState();

	return true;
}

void OutputDecoderState::SetOutput(dcclite::DecoderStates state)
{
	bool active = state == dcclite::DecoderStates::ACTIVE;

	const bool physicalState = this->IsInverted() ? !active : active;
	m_clPin.DigitalWrite(physicalState ? Pin::VHIGH : Pin::VLOW);
}

bool OutputDecoderState::AcceptServerState(dcclite::DecoderStates state)
{	
	return this->SetActive(state);
}

void OutputDecoderState::Init()
{
	if (this->IgnoreSavedState())
	{
		if (this->ActivateOnPowerUp())
			m_fFlags |= dcclite::OUTD_ACTIVE;
		else
			m_fFlags &= ~dcclite::OUTD_ACTIVE;
	}

	this->SetOutput(this->IsActive() ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);
	this->PersistState();
}

void OutputDecoderState::PersistState()
{
	// Index is zero until SaveConfig() assigns the EEPROM slot.
	if (m_uFlagsStorageIndex && !this->IgnoreSavedState())
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);
}



