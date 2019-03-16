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

namespace dcclite
{
	class Guid
	{		
		public:
			//it is public, but access it at your own risk.
			union
			{
				uint8_t m_bId[16];
				uint64_t m_bBigId[2];
			};

			inline Guid()
			{
				m_bBigId[0] = m_bBigId[1] = 0;
			}

			inline bool IsNull() const noexcept
			{
				return (m_bBigId[0] == 0) && (m_bBigId[1] == 0);
			}

			inline bool operator==(const Guid &g) const noexcept
			{
				return ((m_bBigId[0] == g.m_bBigId[0]) && (m_bBigId[1] == g.m_bBigId[1]));
			}

			inline bool operator!=(const Guid &g) const noexcept
			{
				return ((m_bBigId[0] != g.m_bBigId[0]) || (m_bBigId[1] != g.m_bBigId[1]));
			}
	};

}

