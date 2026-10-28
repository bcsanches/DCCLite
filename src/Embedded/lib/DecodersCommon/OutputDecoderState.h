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

#include "Pin.h"

namespace dcclite
{
	class Packet;
}

namespace Storage
{
	class EpromStream;
}

class OutputDecoderState
{
	private:
		uint16_t		m_uFlagsStorageIndex = 0;
		Pin				m_clPin;
		uint8_t			m_fFlags;

	public:
		explicit OutputDecoderState(dcclite::Packet &packet);
		explicit OutputDecoderState(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream);

        inline bool IsActive() const noexcept
        {
            return (m_fFlags & dcclite::OUTD_ACTIVE) != 0;
        }

        inline bool IsInverted() const noexcept
        {
            return (m_fFlags & dcclite::OUTD_INVERTED_OPERATION) != 0;
        }

        inline bool IgnoreSavedState() const noexcept
        {
            return (m_fFlags & dcclite::OUTD_IGNORE_SAVED_STATE) != 0;
        }

        inline bool ActivateOnPowerUp() const noexcept
        {
            return (m_fFlags & dcclite::OUTD_ACTIVATE_ON_POWER_UP) != 0;
        }

        inline void SetUserBits(uint8_t flags) noexcept
        {                       
            //replace user bits...
            m_fFlags = (m_fFlags & ~dcclite::OUTD_USER_BITS) | (flags & dcclite::OUTD_USER_BITS);
        }

        inline uint8_t GetUserBits() const noexcept
        {
            return m_fFlags & dcclite::OUTD_USER_BITS;
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

        void PersistState();
};
