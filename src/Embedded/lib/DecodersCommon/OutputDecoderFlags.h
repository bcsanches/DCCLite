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

#include <cstdint>

#include <dcclite_shared/Packet.h>

#include "Storage.h"

template <
	uint8_t IGNORE_SAVED_STATE_MASK,
	uint8_t INVERTED_MASK,
	uint8_t ACTIVATE_ON_POWER_UP_MASK,
	uint8_t ACTIVE_MASK,
	uint8_t USER_BITS_MASK = 0x00,
	uint8_t PACKET_MASK = 0xFF,
	uint8_t STREAM_MASK = 0xFF
>
struct OutputDecoderFlags
{
	private:
		uint16_t    m_uStorageIndex;
		uint8_t     m_fFlags;

	public:
		inline OutputDecoderFlags() noexcept:
			m_uStorageIndex{0},
			m_fFlags{0}
		{
		}

		inline OutputDecoderFlags(dcclite::Packet &packet) noexcept:
			m_uStorageIndex{0}
		{
			this->Load(packet);
		}

		inline OutputDecoderFlags(Storage::EpromStream &stream) noexcept:
			m_uStorageIndex{ static_cast<uint16_t>(stream.GetIndex()) }
		{
			static_assert(STREAM_MASK != 0, "STREAM_MASK must be non-zero to use OutputDecoderFlags with EpromStream");

			stream.Get(m_fFlags);

			m_fFlags &= STREAM_MASK;
		}

		inline void Load(dcclite::Packet &packet) noexcept
		{
			static_assert(PACKET_MASK != 0, "PACKET_MASK must be non-zero to use OutputDecoderFlags with Packet");
			m_fFlags = packet.Read<uint8_t>() & PACKET_MASK;
		}

		inline void Load(Storage::EpromStream &stream) noexcept
		{
			static_assert(STREAM_MASK != 0, "STREAM_MASK must be non-zero to use OutputDecoderFlags with EpromStream");

			m_uStorageIndex = stream.GetIndex();
			stream.Get(m_fFlags);

			m_fFlags &= STREAM_MASK;
		}

		inline void Save(Storage::EpromStream &stream) noexcept
		{
			static_assert(STREAM_MASK != 0, "STREAM_MASK must be non-zero to use OutputDecoderFlags with EpromStream");

			m_uStorageIndex = stream.GetIndex();
			stream.Put(m_fFlags);
		}

		inline void PersistState() noexcept
		{
			if (m_uStorageIndex && !this->IgnoreSavedState())
				Storage::UpdateField(m_uStorageIndex, m_fFlags);
		}

		inline bool IgnoreSavedState() const noexcept
		{
			static_assert(IGNORE_SAVED_STATE_MASK != 0, "IGNORE_SAVED_STATE_MASK must be non-zero to use IgnoreSavedState");

			return IGNORE_SAVED_STATE_MASK && (m_fFlags & IGNORE_SAVED_STATE_MASK);
		}

		inline bool IsInverted() const noexcept
		{
			static_assert(INVERTED_MASK != 0, "INVERTED_MASK must be non-zero to use IsInverted");

			return INVERTED_MASK && (m_fFlags & INVERTED_MASK);
		}

		inline bool ActivateOnPowerUp() const noexcept
		{
			static_assert(ACTIVATE_ON_POWER_UP_MASK != 0, "ACTIVATE_ON_POWER_UP_MASK must be non-zero to use ActivateOnPowerUp");

			return ACTIVATE_ON_POWER_UP_MASK && (m_fFlags & ACTIVATE_ON_POWER_UP_MASK);
		}

		inline bool IsActive() const noexcept
		{
			static_assert(ACTIVE_MASK != 0, "ACTIVE_MASK must be non-zero to use IsActive");

			return ACTIVE_MASK && (m_fFlags & ACTIVE_MASK);
		}		

		inline void SetActiveBit(bool active) noexcept
		{
			static_assert(ACTIVE_MASK != 0, "ACTIVE_MASK must be non-zero to use SetActiveBit");

			if (active)
				m_fFlags |= ACTIVE_MASK;
			else
				m_fFlags &= ~ACTIVE_MASK;			
		}

		inline void SetUserBits(uint8_t flags) noexcept
		{
			static_assert(USER_BITS_MASK != 0, "USER_BITS_MASK must be non-zero to use SetUserBits");

			//replace user bits...
			m_fFlags = (m_fFlags & ~USER_BITS_MASK) | (flags & USER_BITS_MASK);
		}

		inline uint8_t GetUserBits() const noexcept
		{
			static_assert(USER_BITS_MASK != 0, "USER_BITS_MASK must be non-zero to use SetUserBits");

			return m_fFlags & USER_BITS_MASK;
		}
};
