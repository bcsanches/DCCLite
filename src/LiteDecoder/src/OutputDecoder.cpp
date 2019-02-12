#include "OutputDecoder.h"

#include <Arduino.h>
#include <EmbeddedLibDefs.h>
#include <Packet.h>

#include "Storage.h"

OutputDecoder::OutputDecoder(dcclite::Packet &packet)
{
	m_tPin = packet.Read<Pin_t>();
	m_fFlags = packet.Read<uint8_t>();

	pinMode(m_tPin, OUTPUT);

	using namespace dcclite;

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
	//m_fStatus = bitRead(m_fFlags, 1) ? bitRead(m_fFlags, 2) : 0;
	m_fFlags |= ((m_fFlags & OUTD_IGNORE_SAVED_STATE) ? (m_fFlags & OUTD_ACTIVATE_ON_POWER_UP) : 0) ? OUTD_ACTIVE : 0;
	
	digitalWrite(m_tPin, (m_fFlags & OUTD_ACTIVE) ^ (m_fFlags & OUTD_INVERTED_OPERATION));
}

void OutputDecoder::SaveConfig(EpromStream &stream)
{
	stream.Put(m_tPin);
	stream.Put(m_fFlags);
}
