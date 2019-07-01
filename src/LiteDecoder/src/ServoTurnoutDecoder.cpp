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
	Decoder::Decoder(packet),
	m_clPin{packet.Read<dcclite::PinType_t>()}
{	
	m_fFlags = packet.Read<uint8_t>();

	auto powerPin = packet.Read<dcclite::PinType_t>();	
	auto frogPin = packet.Read<dcclite::PinType_t>();
	
	m_uRange = packet.Read<uint8_t>();
	m_uTicks = packet.Read<uint8_t>();

	this->Init(powerPin, frogPin);
}

ServoTurnoutDecoder::ServoTurnoutDecoder(EpromStream& stream) :
	Decoder::Decoder(stream)
{	
	dcclite::PinType_t pin;
	stream.Get(pin);

	m_clPin = dcclite::BasicPin{ pin };

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Get(m_fFlags);

	dcclite::PinType_t powerPin;
	stream.Get(powerPin);

	dcclite::PinType_t frogPin;
	stream.Get(frogPin);	

	stream.Get(m_uRange);
	stream.Get(m_uTicks);	

	this->Init(powerPin, frogPin);
}

ServoTurnoutDecoder::~ServoTurnoutDecoder()
{
	m_clServo.detach();
}


void ServoTurnoutDecoder::SaveConfig(EpromStream& stream)
{
	Decoder::SaveConfig(stream);

	stream.Put(m_clPin.Raw());

	m_uFlagsStorageIndex = stream.GetIndex();
	stream.Put(m_fFlags);

	stream.Put(m_clPowerPin.Raw());
	stream.Put(m_clFrogPin.Raw());
	
	stream.Put(m_uRange);
	stream.Put(m_uTicks);
}

void ServoTurnoutDecoder::OperatePin()
{
	using namespace dcclite;	

	bool active = (m_fFlags & SRVT_ACTIVE);
	active = (m_fFlags & SRVT_INVERTED_OPERATION) ? !active : active;

	if(m_clPowerPin)
	{
		m_clPowerPin.DigitalWrite(Pin::VLOW);
		m_uNextThink = millis() + 500;
	}

	m_clServo.write(active ? dcclite::SERVO_DEFAULT_RANGE : 0);

	if (m_clFrogPin)
	{
		m_clFrogPin.DigitalWrite(active ? Pin::VHIGH : Pin::VLOW);
	}

	//m_clServo.writeMicroseconds(active ? 0 : 1300); 
	//Console::SendLogEx("ServoTurnoutDecoder", "Operate Pin", active ? 90 : 0);

	//Store current state on eprom, so we can reload.
	if (m_uFlagsStorageIndex)
		Storage::UpdateField(m_uFlagsStorageIndex, m_fFlags);

#if 0
	Console::SendLogEx("[ServoTurnout]", ' ', "PIN: ", m_tPin);
	Console::SendLogEx("[ServoTurnout]", ' ', "IGNORE_SAVE: ", m_fFlags & SRVT_IGNORE_SAVED_STATE);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVATE_ON_POWERUP: ", m_fFlags & SRVT_ACTIVATE_ON_POWER_UP);
	Console::SendLogEx("[ServoTurnout]", ' ', "ACTIVE: ", m_fFlags & SRVT_ACTIVE);
	Console::SendLogEx("[ServoTurnout]", ' ', "INVERTED: ", m_fFlags & SRVT_INVERTED_OPERATION);
#endif
}

void ServoTurnoutDecoder::Init(const dcclite::PinType_t powerPin, const dcclite::PinType_t frogPin)
{
	using namespace dcclite;
	
	m_clServo.attach(m_clPin.Raw());
	
	if(!dcclite::IsPinNull(powerPin))
	{
		m_clPowerPin.Attach(powerPin, Pin::MODE_OUTPUT);
		m_clPowerPin.DigitalWrite(Pin::VHIGH);
	}
	
	if(!dcclite::IsPinNull(frogPin))
	{
		m_clFrogPin.Attach(frogPin, Pin::MODE_OUTPUT);		
	}

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	if (m_fFlags & SRVT_IGNORE_SAVED_STATE)
	{
		if (m_fFlags & SRVT_ACTIVATE_ON_POWER_UP)
			m_fFlags |= SRVT_ACTIVE;
		else
			m_fFlags &= ~SRVT_ACTIVE;
	}

#if 0
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

bool ServoTurnoutDecoder::Update(const unsigned long ticks)
{
	Decoder::Update(ticks);

	if((m_uNextThink <= ticks) && (m_clPowerPin))
	{
		m_clPowerPin.DigitalWrite(Pin::VHIGH);
	}

	return false;
}
