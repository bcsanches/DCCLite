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

#include <array>
#include <ranges>
#include <stdexcept>

namespace LitePanel
{
	static constexpr std::array<std::array<ObjectAngles, 3>, 8> g_kValidAngles =
	{ 
		{
			//Straight					RightCurve					LeftCurve
			{ ObjectAngles::WEST,		ObjectAngles::NORTHWEST,	ObjectAngles::SOUTHWEST }, // From EAST

			//Straight Diagonal			Right Curve					Left Curve
			{ ObjectAngles::SOUTHWEST,	ObjectAngles::WEST,			ObjectAngles::SOUTH }, // From NORTHEAST
			
			//Straight					Left Curve					Right Curve
			{ ObjectAngles::SOUTH,		ObjectAngles::SOUTHEAST,	ObjectAngles::SOUTHWEST }, // From NORTH

			//Straight Diagonal			Left Curve					Right Curve
			{ ObjectAngles::SOUTHEAST,	ObjectAngles::EAST,			ObjectAngles::SOUTH },	// From NORTHWEST

			//Straight					Left Curve					Right Curve
			{ ObjectAngles::EAST,		ObjectAngles::NORTHEAST,	ObjectAngles::SOUTHEAST },	// From WEST
			
			//Straight Diagonal			Right Curve					Left Curve
			{ ObjectAngles::NORTHEAST,	ObjectAngles::EAST,			ObjectAngles::NORTH }, // From SOUTHWEST

			//Straight					Left Curve					Right Curve
			{ ObjectAngles::NORTH,		ObjectAngles::NORTHWEST,	ObjectAngles::NORTHEAST }, // From SOUTH

			//Straight Diagonal			Right Curve					Left Curve
			{ ObjectAngles::NORTHWEST,	ObjectAngles::WEST,			ObjectAngles::NORTH } // From SOUTHEAST
		} 
	};

	RailLink::RailLink(ObjectAngles previous, ObjectAngles next) :
		m_kPrevious(previous),
		m_kNext(next)
	{
		if (m_kPrevious == m_kNext)
		{
			throw std::invalid_argument("[RailLink] Previous and next angles cannot be equal");
		}

		const auto previousIndex = static_cast<int>(m_kPrevious);

		if (previousIndex >= g_kValidAngles.size())
		{
			throw std::invalid_argument("[RailLink] Invalid previous angle");
		}

		auto it = std::ranges::find(g_kValidAngles[previousIndex], m_kNext);
		if(it == g_kValidAngles[previousIndex].end())
			throw std::invalid_argument("[RailLink] Previous and next angles form invalid shape");
	}

	RailTypes RailLink::GetType() const noexcept
	{
		if (this->IsStraight())
			return RailTypes::STRAIGHT;

		static constexpr std::array<ObjectAngles, 8> leftCurves =
		{
			ObjectAngles::SOUTHWEST,	// EAST
			ObjectAngles::SOUTH,		// NORTHEAST
			ObjectAngles::SOUTHEAST,	// NORTH
			ObjectAngles::EAST,			// NORTHWEST
			ObjectAngles::NORTHEAST,	// WEST
			ObjectAngles::NORTH,		// SOUTHWEST
			ObjectAngles::NORTHWEST,	// SOUTH
			ObjectAngles::NORTH			// SOUTHEAST
		};

		const auto index = static_cast<unsigned>(m_kPrevious);

		return (m_kNext == leftCurves[index])
			? RailTypes::CURVE_LEFT
			: RailTypes::CURVE_RIGHT;
	}
}
