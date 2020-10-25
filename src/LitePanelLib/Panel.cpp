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
		kRAIL_LAYER,
		kTEMP_LAYER,

		kNUM_LAYERS
	};

	Panel::Panel(const TileCoord_t size):
		m_mapTileMap(size, kNUM_LAYERS)
	{
		//empty
	}

	void Panel::RegisterRail(std::unique_ptr<RailObject> object)
	{
		m_mapTileMap.RegisterObject(std::move(object), kRAIL_LAYER);
	}

	void Panel::RegisterTempObject(std::unique_ptr<MapObject> object)
	{
		m_mapTileMap.RegisterObject(std::move(object), kTEMP_LAYER);
	}
}
