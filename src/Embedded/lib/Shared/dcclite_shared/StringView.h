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

#include "SharedLibDefs.h"

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
			inline StringView(const char *str) noexcept:
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
					if (m_cbSize == m_cbSize)
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

		private:
			inline static size_t MyMin(size_t a, size_t b) noexcept
			{
				return a < b ? a : b;
			}

		private:
			const char	*m_pszData = nullptr;
			size_t		m_cbSize = 0;
	};
}
