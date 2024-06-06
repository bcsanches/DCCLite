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
	struct Point
	{
		typedef T Type_t;

		T m_tX = { 0 }, m_tY = { 0 };

		Point() = default;
		Point(const Point &) = default;

		template <typename Y>
		inline Point(const Point<Y> &rhs) noexcept:
			m_tX{rhs.m_tX},
			m_tY{rhs.m_tY}
		{
			//empty
		}

		inline Point(T x, T y) noexcept:
			m_tX(x),
			m_tY(y)
		{
			//empty
		}

		inline const Point operator/(const T num) const noexcept
		{
			return Point(m_tX / num, m_tY / num);
		}
		
		inline const Point operator*(const T num) const noexcept
		{
			return Point{ m_tX * num, m_tY * num };
		}

		inline const Point operator*(const Point &rhs) const noexcept
		{
			return Point{ m_tX * rhs.m_tX, m_tY * rhs.m_tY };
		}

		inline const Point operator+(const Point &rhs) const noexcept
		{
			return Point(m_tX + rhs.m_tX, m_tY + rhs.m_tY);
		}

		inline const Point operator-(const Point &rhs) const noexcept
		{
			return Point(m_tX - rhs.m_tX, m_tY - rhs.m_tY);
		}

		inline const Point &operator+=(const Point &rhs) noexcept
		{
			m_tX += rhs.m_tX;
			m_tY += rhs.m_tY;

			return *this;
		}

		inline bool operator==(const Point& rhs) const noexcept
		{
			return (m_tX == rhs.m_tX) && (m_tY == rhs.m_tY);
		}

		inline bool operator!=(const Point &rhs) const noexcept
		{
			return (m_tX != rhs.m_tX) || (m_tY != rhs.m_tY);
		}
	};

	typedef Point<int_fast32_t> IntPoint_t;
	typedef Point<float> FloatPoint_t;
}
