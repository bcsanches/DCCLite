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
	class RailObject: public MapObject
	{
		public:
			RailObject(const TileCoord_t &position);	

		private:
			ObjectAngles m_tAngle = ObjectAngles::EAST;
	};

	enum class SimpleRailTypes
	{
		STRAIGHT,
		LEFT_TURN,
		RIGHT_TURN,
		CROSSING
	};

	class SimpleRailObject: public RailObject
	{
		public:
			SimpleRailObject(const TileCoord_t &position, const SimpleRailTypes type);

		private:
			const SimpleRailTypes m_tType;
	};

	enum class JunctionTypes
	{
		LEFT_TURNOUT,
		RIGHT_TURNOUT
	};

	class JunctionRailObject: public RailObject
	{
		public:
			JunctionRailObject(const TileCoord_t &position, const JunctionTypes type);

		private:
			const JunctionTypes m_tType;
	};
}
