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

#include "TileMap.h"

namespace LitePanel
{
	class RailObject;

	class Panel
	{
		public:
			Panel(const TileCoord_t size);

			void RegisterRail(std::unique_ptr<RailObject> object);
			std::unique_ptr<RailObject> TryUnregisterRail(const TileCoord_t &position);

			bool IsRailTileOccupied(const TileCoord_t &position) const noexcept;

			void RegisterTempObject(std::unique_ptr<MapObject> object);

			void SetTempObjectPosition(MapObject &obj, const TileCoord_t &newPosition);
			std::unique_ptr<MapObject> UnregisterTempObject(const MapObject &obj);

			inline TileMap& GetTileMap() noexcept
			{
				return m_mapTileMap;
			}

		private:
			TileMap m_mapTileMap;
			
	};
}
