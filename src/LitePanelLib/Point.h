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

		Point(T x, T y):
			m_tX(x),
			m_tY(y)
		{
			//empty
		}

		const Point operator/(T num) const
		{
			return Point(m_tX / num, m_tY / num);
		}

		const Point operator*(T num) const
		{
			return Point{ m_tX * num, m_tY * num };
		}

		const Point operator+(const Point &rhs) const
		{
			return Point(m_tX + rhs.m_tX, m_tY + rhs.m_tY);
		}

		const Point operator-(const Point &rhs) const
		{
			return Point(m_tX - rhs.m_tX, m_tY - rhs.m_tY);
		}

		const Point &operator+=(const Point &rhs)
		{
			m_tX += rhs.m_tX;
			m_tY += rhs.m_tY;

			return *this;
		}
	};

	typedef Point<int_fast32_t> IntPoint_t;
}
