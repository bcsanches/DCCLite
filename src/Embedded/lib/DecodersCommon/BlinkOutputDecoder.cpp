// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "BlinkOutputDecoder.h"

#include <dcclite_shared/Packet.h>

#include "Storage.h"

BlinkOutputDecoder::BlinkOutputDecoder(dcclite::Packet &packet) :
	Decoder{packet},
	m_clState{ packet }
{
	m_u16IntervalMs = packet.Read<uint16_t>();

	this->Init();
}

BlinkOutputDecoder::BlinkOutputDecoder(Storage::EpromStream &stream):
	Decoder{stream},
	m_clState{stream}
{
	stream.Get(m_u16IntervalMs);

	this->Init();
}

void BlinkOutputDecoder::SaveConfig(Storage::EpromStream &stream)
{
	Decoder::SaveConfig(stream);

	m_clState.SaveConfig(stream);

	stream.Put(m_u16IntervalMs);
}

/// <summary>
/// 
/// </summary>
/// <param name="ticks"></param>
/// <returns>We always return false, we do not want to bother broker with the blinking...</returns>
bool BlinkOutputDecoder::Update(const unsigned long ticks)
{
	if (!this->IsActive())
		return false;

	if (ticks < m_uNextThink)
		return false;

	auto bits = m_clState.GetUserBits();

	//are we blinking?
	if(!(bits & dcclite::BOUTD_BLINKING))
		return false;

	//Flip state...
	bool outputActive = bits & dcclite::BOUTD_OUTPUT_ACTIVE;
	if (outputActive)
	{
		bits &= ~dcclite::BOUTD_OUTPUT_ACTIVE;
		outputActive = false;
	}
	else
	{
		bits |= dcclite::BOUTD_OUTPUT_ACTIVE;
		outputActive = true;
	}	

	//Calculate next think...
	m_uNextThink += m_u16IntervalMs;

	//Now change physical state
	m_clState.SetOutput(outputActive ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);

	//store bits...
	m_clState.SetUserBits(bits);

	return false;
}

bool BlinkOutputDecoder::AcceptServerState(dcclite::DecoderStates state, const unsigned long time)
{
	const auto stageChange = m_clState.AcceptServerState(state);

	if ((stageChange) && (state == dcclite::DecoderStates::ACTIVE) && (m_clState.GetUserBits() & dcclite::BOUTD_BLINKING))
		this->StartBlinking(time);

	return stageChange;
}

void BlinkOutputDecoder::StartBlinking(const unsigned long time)
{
	m_clState.SetUserBits(dcclite::BOUTD_BLINKING);
	m_uNextThink = time + m_u16IntervalMs;
}

void BlinkOutputDecoder::Init()
{
	if(!(m_clState.GetUserBits() &dcclite::BOUTD_BLINKING))
		return;

	this->StartBlinking(millis());
}

void BlinkOutputDecoder::SetBlinkingState(dcclite::DecoderStates state)
{
	if (state == dcclite::DecoderStates::ACTIVE)
	{
		this->StartBlinking(millis());
	}
	else
	{
		m_clState.SetUserBits(0);
		m_clState.SetOutput(m_clState.IsActive() ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE);		
	}
}

