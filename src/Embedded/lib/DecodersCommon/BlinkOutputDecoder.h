// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Decoder.h"

#include "OutputDecoderState.h"

class BlinkOutputDecoder : public Decoder
{
	private:
		OutputDecoderState	m_clState;

		uint16_t			m_u16IntervalMs;
		unsigned long		m_uNextThink = 0;

	public:
		explicit BlinkOutputDecoder(dcclite::Packet &packet);
		explicit BlinkOutputDecoder(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream) override;

		bool IsOutputDecoder() const override
		{
			return true;
		}

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_BLINK_OUTPUT;
		};

		bool AcceptServerState(dcclite::DecoderStates state, const unsigned long time) override;

		bool IsActive() const override
		{
			return m_clState.IsActive();
		}

		inline bool IsInverted() const
		{
			return m_clState.IsInverted();
		}

		bool IsSyncRequired() const override
		{
			return false;
		}
		
		bool Update(const unsigned long ticks) override;

		void SetBlinkingState(dcclite::DecoderStates state);
	
	private:
		void Init();

		void StartBlinking(const unsigned long time);
};
