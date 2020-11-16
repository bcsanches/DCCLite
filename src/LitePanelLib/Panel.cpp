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
		kTEMP_LAYER,
		kRAIL_LAYER,		

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

	std::unique_ptr<RailObject> Panel::TryUnregisterRail(const TileCoord_t &position)
	{
		auto tmp = m_mapTileMap.TryUnregisterObject(position, kRAIL_LAYER);

		return std::unique_ptr<RailObject>(static_cast<RailObject *>(tmp.release()));
	}

	bool Panel::IsRailTileOccupied(const TileCoord_t &position) const noexcept
	{
		return m_mapTileMap.IsTileOccupied(position, kRAIL_LAYER);
	}

	void Panel::RegisterTempObject(std::unique_ptr<MapObject> object)
	{
		m_mapTileMap.RegisterObject(std::move(object), kTEMP_LAYER);
	}

	std::unique_ptr<MapObject> Panel::UnregisterTempObject(const MapObject& object)
	{
		return m_mapTileMap.UnregisterObject(object, kTEMP_LAYER);
	}

	void Panel::SetTempObjectPosition(MapObject& object, const TileCoord_t& newPosition)
	{		
		auto obj = this->UnregisterTempObject(object);
		
		obj->SetPosition(newPosition);

		this->RegisterTempObject(std::move(obj));
	}

	void Panel::Save(JsonOutputStream_t &stream) const
	{
		auto tileMapObj = stream.AddObject("tileMap");

		m_mapTileMap.Save(tileMapObj);
	}

}
