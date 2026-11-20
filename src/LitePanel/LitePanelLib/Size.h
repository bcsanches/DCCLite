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

#include <cstdint>

namespace LitePanel
{
	template <typename T>
	struct Size
	{
		typedef T Type_t;

		T m_tWidth = { 0 };
		T m_tHeight = { 0 };

		Size() = default;
		Size(const Size &) = default;

		template <typename Y>
		inline Size(const Size<Y> &rhs) noexcept :
			m_tWidth{ rhs.m_tWidth },
			m_tHeight{ rhs.m_tHeight }
		{
			//empty
		}

		inline Size(T width, T height) noexcept :
			m_tWidth{ width },
			m_tHeight{ height }
		{
			//empty
		}

		inline Size(const Point<T> &p) noexcept :
			m_tWidth{ p.m_tX },
			m_tHeight{ p.m_tY }
		{
			//empty
		}

		inline Size operator*(T value) const noexcept
		{
			return { m_tWidth * value, m_tHeight * value };
		}

		inline Size operator/(T value) const noexcept
		{
			return { m_tWidth / value, m_tHeight / value };
		}

		inline Size operator+(const Size &rhs) const noexcept
		{
			return {
				m_tWidth + rhs.m_tWidth,
				m_tHeight + rhs.m_tHeight
			};
		}

		inline Size operator-(const Size &rhs) const noexcept
		{
			return {
				m_tWidth - rhs.m_tWidth,
				m_tHeight - rhs.m_tHeight
			};
		}

		inline bool operator==(const Size &rhs) const noexcept
		{
			return m_tWidth == rhs.m_tWidth && m_tHeight == rhs.m_tHeight;
		}

		inline bool operator!=(const Size &rhs) const noexcept
		{
			return !(*this == rhs);
		}
	};

	typedef Size<int_fast32_t> IntSize_t;
	typedef Size<float> FloatSize_t;
}
