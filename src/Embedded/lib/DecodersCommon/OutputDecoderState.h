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

#include "OutputDecoderFlags.h"
#include "Pin.h"

namespace dcclite
{
	class Packet;
}

namespace Storage
{
	class EpromStream;
}

using StandardOutputDecoderFlags = OutputDecoderFlags<
	dcclite::OUTD_IGNORE_SAVED_STATE,
	dcclite::OUTD_INVERTED_OPERATION,
	dcclite::OUTD_ACTIVATE_ON_POWER_UP,
	dcclite::OUTD_ACTIVE,
	dcclite::OUTD_USER_BITS
>;

class OutputDecoderState
{
	private:
		StandardOutputDecoderFlags  m_fFlags;

		Pin				            m_clPin;		

	public:
		explicit OutputDecoderState(dcclite::Packet &packet);
		explicit OutputDecoderState(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream);

		inline bool IsActive() const noexcept
		{
			return m_fFlags.IsActive();
		}

		inline bool IsInverted() const noexcept
		{
			return m_fFlags.IsInverted();            
		}

		inline bool IgnoreSavedState() const noexcept
		{
			return m_fFlags.IgnoreSavedState();            
		}

		inline bool ActivateOnPowerUp() const noexcept
		{
			return m_fFlags.ActivateOnPowerUp();            
		}

		inline void SetUserBits(uint8_t flags) noexcept
		{   
			m_fFlags.SetUserBits(flags);
		}

		inline uint8_t GetUserBits() const noexcept
		{
			return m_fFlags.GetUserBits();
		}

		// Changes logical state and persists it when configured to do so.
		// Returns false if the logical state did not change.
		bool SetActive(dcclite::DecoderStates state);

		// Changes only the physical pin level (with inversion). Useful for blink
		// phases: the decoder can remain logically active while the pin is off.
		void SetOutput(dcclite::DecoderStates state);

		bool AcceptServerState(dcclite::DecoderStates state);

	private:
		void Init();
};
