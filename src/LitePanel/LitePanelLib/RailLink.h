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

#include "MapObject.h"

namespace LitePanel
{
	enum class RailTypes
	{
		STRAIGHT,
		CURVE_LEFT,
		CURVE_RIGHT		
	};

	class RailLink
	{
		public:
			RailLink(ObjectAngles previous, ObjectAngles next);

			ObjectAngles GetPrevious() const noexcept
			{
				return m_kPrevious;
			}

			ObjectAngles GetNext() const noexcept
			{
				return m_kNext;
			}

			bool constexpr IsStraight() const noexcept
			{
				return (static_cast<unsigned>(m_kPrevious) + 4u) % 8u == static_cast<unsigned>(m_kNext);
			}

			RailTypes GetType() const noexcept;

			bool operator==(const RailLink &) const = default;

		private:
			const ObjectAngles m_kPrevious;
			const ObjectAngles m_kNext;
	};
}
