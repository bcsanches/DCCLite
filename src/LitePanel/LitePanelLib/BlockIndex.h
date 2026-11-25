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
#include <limits>

namespace LitePanel
{
	class BlockIndex
	{
		public:
			using Value_t = std::uint32_t;

			static constexpr Value_t NULL_VALUE = std::numeric_limits<Value_t>::max();

			constexpr BlockIndex() noexcept = default;

			constexpr BlockIndex(std::int64_t value) noexcept :
				m_tValue{
					(value < 0 || value >= NULL_VALUE) ?
						NULL_VALUE :
						static_cast<Value_t>(value)
			}
			{
			}

			constexpr bool IsNull() const noexcept
			{
				return m_tValue == NULL_VALUE;
			}

			constexpr explicit operator bool() const noexcept
			{
				return !IsNull();
			}

			constexpr Value_t GetValue() const noexcept
			{
				return m_tValue;
			}

			constexpr bool operator==(const BlockIndex &) const noexcept = default;

		private:
			Value_t m_tValue = NULL_VALUE;
	};
}
