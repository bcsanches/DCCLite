// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "RailObject.h"

namespace LitePanel
{	
	RailObject::RailObject(const TileCoord_t &position, ObjectAngles angle):
		MapObject(position),
		m_tAngle(angle)
	{
		//empty
	}	

	SimpleRailObject::SimpleRailObject(const TileCoord_t &position, ObjectAngles angle, const SimpleRailTypes type):
		RailObject(position, angle),
		m_tType(type)
	{
		//empty
	}

	JunctionRailObject::JunctionRailObject(const TileCoord_t &position, ObjectAngles angle, const JunctionTypes type):
		RailObject(position, angle),
		m_tType(type)
	{
		//empty
	}
}
