// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <string.h>

namespace dcclite
{
	template<size_t NBITS>
	class BitPack
	{
		private:
			static_assert((NBITS % 8) == 0, "NBITS must be multiple of 8");
			unsigned char m_bData[NBITS / 8] = { 0 };

			unsigned BytePosition(const unsigned index) const
			{
				return index / 8;
			}

			unsigned Shift(const unsigned index) const
			{
				return index % 8;
			}

		public:
			bool operator==(const BitPack<NBITS>& rhs) const
			{
				return memcmp(m_bData, rhs.m_bData, sizeof(m_bData)) == 0;
			}

			bool operator[](const unsigned index) const
			{
				const int bytePos = BytePosition(index);

				const int shift = Shift(index);

				return (m_bData[bytePos] >> shift) & 0x01;
			}

			void SetBit(const unsigned index)
			{
				const int bytePos = BytePosition(index);

				const int shift = Shift(index);

				m_bData[bytePos] |= 0x01 << shift;
			}

			void ClearBit(const unsigned index)
			{
				const int bytePos = BytePosition(index);

				const int shift = Shift(index);

				m_bData[bytePos] &= ~(0x01 << shift);
			}

			inline void SetBitValue(const unsigned index, bool value)
			{
				value ? SetBit(index) : ClearBit(index);
			}

			inline void ClearAll()
			{
				memset(m_bData, 0, sizeof(m_bData));
			}

			inline unsigned size() const
			{
				return NBITS;
			}

			inline const uint8_t *GetRaw() const { return m_bData; }

			inline void Set(const void *data)
			{
				memcpy(m_bData, data, sizeof(m_bData));
			}

			inline unsigned GetNumBytes() const
			{
				return NBITS / 8;
			}
	};
}
