// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TileMap.h"

#include <stdexcept>

#include <fmt/format.h>

#include "MapObject.h"

namespace LitePanel
{

	TileMap::TileMap(const TileCoord_t size):
		m_tSize{size},
		m_vecMap{size.m_tX * size.m_tY}
	{
		if ((size.m_tX == 0) || (size.m_tY == 0))
			throw std::invalid_argument("[LitePanel::TileMap] size must be > 0");		
	}

	size_t TileMap::GetIndex(const TileCoord_t &position) const
	{
		if((position.m_tX >= m_tSize.m_tX) || (position.m_tY >= m_tSize.m_tY))
			throw std::out_of_range(fmt::format("[TileMap::CheckCoordinate] Coordinate {} - {} is out of bounds for size {} - {]", 
				position.m_tX,
				position.m_tY,
				m_tSize.m_tX,
				m_tSize.m_tY
			));

		return (position.m_tY * m_tSize.m_tY) + position.m_tX;
	}

	void TileMap::RegisterObject(std::unique_ptr<MapObject> object)
	{
		auto index = this->GetIndex(object->GetPosition());

		if (m_vecMap[index])
		{
			const auto pos = object->GetPosition();

			throw std::runtime_error(fmt::format("[TileMap::RegisterObject] Position {} - {} is occupied", pos.m_tX, pos.m_tY));
		}

		std::swap(m_vecMap[index], object);
	}
}
