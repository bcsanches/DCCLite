// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "RailLink.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

#include <gtest/gtest.h>

namespace LitePanel
{
	namespace
	{
		using AnglePair_t = std::pair<ObjectAngles, ObjectAngles>;

		constexpr std::array<AnglePair_t, 24> g_kValidLinks =
		{{
			// From EAST
			{ ObjectAngles::EAST, ObjectAngles::WEST },
			{ ObjectAngles::EAST, ObjectAngles::NORTHWEST },
			{ ObjectAngles::EAST, ObjectAngles::SOUTHWEST },

			// From NORTHEAST
			{ ObjectAngles::NORTHEAST, ObjectAngles::SOUTHWEST },
			{ ObjectAngles::NORTHEAST, ObjectAngles::WEST },
			{ ObjectAngles::NORTHEAST, ObjectAngles::SOUTH },

			// From NORTH
			{ ObjectAngles::NORTH, ObjectAngles::SOUTH },
			{ ObjectAngles::NORTH, ObjectAngles::SOUTHEAST },
			{ ObjectAngles::NORTH, ObjectAngles::SOUTHWEST },

			// From NORTHWEST
			{ ObjectAngles::NORTHWEST, ObjectAngles::SOUTHEAST },
			{ ObjectAngles::NORTHWEST, ObjectAngles::EAST },
			{ ObjectAngles::NORTHWEST, ObjectAngles::SOUTH },

			// From WEST
			{ ObjectAngles::WEST, ObjectAngles::EAST },
			{ ObjectAngles::WEST, ObjectAngles::NORTHEAST },
			{ ObjectAngles::WEST, ObjectAngles::SOUTHEAST },

			// From SOUTHWEST
			{ ObjectAngles::SOUTHWEST, ObjectAngles::NORTHEAST },
			{ ObjectAngles::SOUTHWEST, ObjectAngles::EAST },
			{ ObjectAngles::SOUTHWEST, ObjectAngles::NORTH },

			// From SOUTH
			{ ObjectAngles::SOUTH, ObjectAngles::NORTH },
			{ ObjectAngles::SOUTH, ObjectAngles::NORTHWEST },
			{ ObjectAngles::SOUTH, ObjectAngles::NORTHEAST },

			// From SOUTHEAST
			{ ObjectAngles::SOUTHEAST, ObjectAngles::NORTHWEST },
			{ ObjectAngles::SOUTHEAST, ObjectAngles::WEST },
			{ ObjectAngles::SOUTHEAST, ObjectAngles::NORTH }
		}};
	}

	TEST(RailLinkTest, AcceptsAllValidAnglePairs)
	{
		for (const auto &[previous, next] : g_kValidLinks)
		{
			const RailLink link{ previous, next };

			EXPECT_EQ(link.GetPrevious(), previous);
			EXPECT_EQ(link.GetNext(), next);
		}
	}

	TEST(RailLinkTest, RejectsEqualAngles)
	{
		for (int angle = 0; angle < 8; ++angle)
		{
			const auto value = static_cast<ObjectAngles>(angle);

			EXPECT_THROW(RailLink(value, value), std::invalid_argument)
				<< "angle=" << angle;
		}
	}

	TEST(RailLinkTest, RejectsInvalidAngleCombinations)
	{
		for (int previous = 0; previous < 8; ++previous)
		{
			for (int next = 0; next < 8; ++next)
			{
				const AnglePair_t pair{
					static_cast<ObjectAngles>(previous),
					static_cast<ObjectAngles>(next)
				};

				const bool isValid = std::ranges::find(g_kValidLinks, pair) != g_kValidLinks.end();
				if (isValid)
					continue;

				EXPECT_THROW(RailLink(pair.first, pair.second), std::invalid_argument)
					<< "previous=" << previous << ", next=" << next;
			}
		}
	}

	TEST(RailLinkTest, RejectsOutOfRangePreviousAngle)
	{
		const auto invalidPrevious = static_cast<ObjectAngles>(8);

		EXPECT_THROW(
			RailLink(invalidPrevious, ObjectAngles::WEST),
			std::invalid_argument
		);
	}

	TEST(RailLinkTest, RejectsOutOfRangeNextAngle)
	{
		const auto invalidNext = static_cast<ObjectAngles>(8);

		EXPECT_THROW(
			RailLink(ObjectAngles::EAST, invalidNext),
			std::invalid_argument
		);
	}
}
