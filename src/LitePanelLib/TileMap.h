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

#include <memory>
#include <vector>

#include "Point.h"

namespace LitePanel
{
	constexpr auto DEFAULT_TILE_SIZE = 32;

	typedef Point<uint8_t> TileCoord_t;

	class MapObject;

	class TileMap
	{
		public:
			TileMap(const TileCoord_t size);

			const TileCoord_t &GetSize() const noexcept { return m_tSize; }

			void RegisterObject(std::unique_ptr<MapObject> object);

		private:
			size_t GetIndex(const TileCoord_t &position) const;

		private:
			TileCoord_t m_tSize;

			std::vector<std::unique_ptr<MapObject>> m_vecMap;

	};
}
