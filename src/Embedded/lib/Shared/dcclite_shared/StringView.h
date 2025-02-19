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

#include "Misc.h"
#include "SharedLibDefs.h"

#ifdef DCCLITE_DESKTOP
#include <string>
#include <string_view>
#endif

namespace dcclite
{
	/**************************************************************************
	* 
	* 
	* Why? Because Arduino mega 2560 does not support std::string_view
	* 
	*
	**************************************************************************/
	class StringView final
	{
		public:
			inline explicit StringView(const char *str) noexcept:
				m_pszData(str),
				m_cbSize(strlen(str))
			{
				assert(str);
			}

			inline StringView(const char *str, size_t sz) noexcept:
				m_pszData(str),
				m_cbSize(sz)
			{
				assert(str);
			}

#ifdef DCCLITE_DESKTOP
			inline explicit StringView(std::string_view sv) noexcept:
				m_pszData{ sv.data() },
				m_cbSize{ sv.size() }
			{
				//empty
			}

			inline explicit StringView(const std::string &str) noexcept:
				m_pszData{ str.data() },
				m_cbSize{ str.size() }
			{
				//empty
			}
#endif

			~StringView() = default;

			inline StringView(const StringView &) = default;
			inline StringView(StringView &&) = default;

			inline StringView &operator=(const StringView &rhs) = default;			

			inline int Compare(const StringView &rhs) const noexcept
			{
				return strncmp(m_pszData, rhs.m_pszData, MyMin(rhs.m_cbSize, m_cbSize));
			}

			inline int Compare(const char *str) const noexcept
			{
				return strncmp(m_pszData, str, m_cbSize);
			}

			inline char operator[](size_t index) const noexcept
			{
				assert(index < m_cbSize);

				return m_pszData[index];
			}

			bool operator==(const StringView &rhs) const noexcept
			{
				if (m_pszData == rhs.m_pszData)
				{
					if (m_cbSize == rhs.m_cbSize)
						return 0;
				}

				return Compare(rhs) == 0;
			}

			inline const char *GetData() const noexcept
			{
				return m_pszData;
			}

			inline size_t GetSize() const noexcept
			{
				return m_cbSize;
			}
#ifdef DCCLITE_DESKTOP
			inline std::string_view ToStdView() const noexcept
			{
				return std::string_view{ m_pszData, m_cbSize };
			}
#endif

		private:
			const char	*m_pszData = nullptr;
			size_t		m_cbSize = 0;
	};
}
