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

#include <assert.h>
#include <stdexcept>

#include <fmt/format.h>

#include "MapObject.h"

namespace LitePanel
{
	TileLayer::TileLayer(const TileCoord_t size):
		m_vecMap{size.m_tX * size.m_tY},
		m_tSize{size}
	{
		assert(size.m_tX > 0);
		assert(size.m_tY > 0);
	}

	TileLayer::TileLayer(TileLayer &&other) noexcept:
		m_tSize(std::move(other.m_tSize)),
		m_vecMap(std::move(other.m_vecMap))
	{
		//empty
	}

	size_t TileLayer::GetIndex(const TileCoord_t &position) const
	{
		if ((position.m_tX >= m_tSize.m_tX) || (position.m_tY >= m_tSize.m_tY))
			throw std::out_of_range(fmt::format("[TileMap::CheckCoordinate] Coordinate {} - {} is out of bounds for size {} - {]",
				position.m_tX,
				position.m_tY,
				m_tSize.m_tX,
				m_tSize.m_tY
			));

		return (position.m_tY * m_tSize.m_tY) + position.m_tX;
	}

	const MapObject *TileLayer::TryGetMapObject(const TileCoord_t position) const
	{
		auto index = this->GetIndex(position);

		return m_vecMap[index].get();
	}

	void TileLayer::RegisterObject(std::unique_ptr<MapObject> object)
	{
		auto index = this->GetIndex(object->GetPosition());

		if (m_vecMap[index])
		{
			const auto pos = object->GetPosition();

			throw std::runtime_error(fmt::format("[TileMap::RegisterObject] Position {} - {} is occupied", pos.m_tX, pos.m_tY));
		}

		std::swap(m_vecMap[index], object);
	}

	std::unique_ptr<MapObject> TileLayer::UnregisterObject(const MapObject& obj)
	{
		auto &pos = obj.GetPosition();
		auto index = this->GetIndex(pos);

		if (m_vecMap[index].get() != &obj)
		{
			throw std::runtime_error(fmt::format("[TileLayer::UnregisterObject] Position {} - {} is not occupied by object", pos.m_tX, pos.m_tY));
		}

		return std::move(m_vecMap[index]);
	}


	TileMap::TileMap(const TileCoord_t size, const unsigned numLayers)
	{
		if ((size.m_tX == 0) || (size.m_tY == 0))
			throw std::invalid_argument("[LitePanel::TileMap] size must be > 0");	

		if(numLayers == 0)
			throw std::invalid_argument("[LitePanel::TileMap] numLayers must be > 0");


		for (unsigned i = 0; i < numLayers; ++i)
		{
			m_vecLayers.emplace_back(size);
		}						
	}

	void TileMap::RegisterObject(std::unique_ptr<MapObject> object, const uint8_t layer)
	{
		if(layer >= m_vecLayers.size())
			throw std::runtime_error(fmt::format("[TileMap::RegisterObject] Layer {} is invalid", layer));

		m_vecLayers[layer].RegisterObject(std::move(object));

		this->StateChanged();
	}

	std::unique_ptr<MapObject> TileMap::UnregisterObject(const MapObject& obj, const uint8_t layer)
	{
		if(layer >= m_vecLayers.size())
			throw std::runtime_error(fmt::format("[TileMap::UnregisterObject] layer {} is invalid", layer));

		auto tmp = m_vecLayers[layer].UnregisterObject(obj);

		this->StateChanged();

		return tmp;
	}

	void TileMap::AddListener(ITileMapListener* listener)
	{
		assert(listener);

		m_vecListeners.push_back(listener);
	}

	void TileMap::RemoveListener(ITileMapListener* listener)
	{
		m_vecListeners.erase(
			std::remove_if(m_vecListeners.begin(), m_vecListeners.end(), [listener](ITileMapListener* obj)
				{
					return obj == listener;
				}
			),
			m_vecListeners.end()
		);
	}

	void TileMap::StateChanged()
	{
		for (auto listener : m_vecListeners)
		{
			listener->TileMap_OnStateChanged();
		}
	}

}
