// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "MapObject.h"
#include "Panel.h"
#include "RailObject.h"

namespace LitePanel
{
	enum Layers
	{
		RAIL_LAYER
	};

	Panel::Panel(const TileCoord_t size):
		m_mapTileMap(size)		
	{
		//empty
	}

	void Panel::RegisterRail(std::unique_ptr<RailObject> object)
	{
		m_mapTileMap.RegisterObject(std::move(object), RAIL_LAYER);
	}
}
