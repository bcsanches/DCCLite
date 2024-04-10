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
		m_vecMap{static_cast<size_t>(size.m_tX * size.m_tY)},
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
			throw std::out_of_range(fmt::format("[TileMap::CheckCoordinate] Coordinate {} - {} is out of bounds for size {} - {}",
				position.m_tX,
				position.m_tY,
				m_tSize.m_tX,
				m_tSize.m_tY
		));

		return (position.m_tY * m_tSize.m_tX) + position.m_tX;
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

	void TileLayer::Save(JsonOutputStream_t &stream) const
	{
		auto outputArray = stream.AddArray("objects");

		for (auto& obj : m_vecMap)
		{
			if (!obj || !obj->IsPersistent())
				continue;
			
			auto outputObj = outputArray.AddObject();
			obj->Save(outputObj);
		}
	}

	void TileLayer::Load(const rapidjson::Value& data)
	{
		auto objectsArray = data["objects"].GetArray();

		for (auto& it : objectsArray)
		{			
			this->RegisterObject(MapObject::Create(it));
		}
	}

	//
	//
	// TileMap
	//
	//

	static void CheckSize(const TileCoord_t size)
	{
		if ((size.m_tX == 0) || (size.m_tY == 0))
			throw std::invalid_argument("[LitePanel::TileMap] size must be > 0");		
	}


	TileMap::TileMap(const TileCoord_t size, const unsigned numLayers)
	{
		CheckSize(size);

		if (numLayers == 0)
			throw std::invalid_argument("[LitePanel::TileMap] numLayers must be > 0");

		for (unsigned i = 0; i < numLayers; ++i)
		{
			m_vecLayers.emplace_back(size);
		}						
	}

	TileMap::TileMap(const rapidjson::Value& data) :
		TileMap(
			TileCoord_t{ static_cast<uint8_t>(data["width"].GetInt()), static_cast<uint8_t>(data["height"].GetInt()) },
			data["layers"].Size()
		)
	{
		auto layersArray = data["layers"].GetArray();

		int index = 0;
		for (auto &it : layersArray)
		{
			m_vecLayers[index].Load(it);
			++index;
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

	std::unique_ptr<MapObject> TileMap::TryUnregisterObject(const TileCoord_t &position, const uint8_t layer)
	{
		if (layer >= m_vecLayers.size())
			throw std::runtime_error(fmt::format("[TileMap::TryUnregisterObject] layer {} is invalid", layer));

		auto obj = m_vecLayers[layer].TryGetMapObject(position);
		if (!obj)
			return nullptr;

		auto tmp = m_vecLayers[layer].UnregisterObject(*obj);

		this->StateChanged();

		return tmp;
	}

	bool TileMap::IsTileOccupied(const TileCoord_t &position, const uint8_t layer) const
	{
		if (layer >= m_vecLayers.size())
			throw std::runtime_error(fmt::format("[TileMap::IsTileOccupied] layer {} is invalid", layer));

		return m_vecLayers[layer].TryGetMapObject(position) != nullptr;
	}

	const MapObject *TileMap::TryGetMapObject(const TileCoord_t pos, const uint8_t layer) const
	{
		if (layer >= m_vecLayers.size())
			throw std::runtime_error(fmt::format("[TileMap::IsTileOccupied] layer {} is invalid", layer));

		return m_vecLayers[layer].TryGetMapObject(pos);
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

	void TileMap::Save(JsonOutputStream_t& stream) const
	{
		const auto &size = m_vecLayers[0].GetSize();

		stream.AddIntValue("width", size.m_tX);
		stream.AddIntValue("height", size.m_tY);

		auto layersArray = stream.AddArray("layers");
		for (auto& layer : m_vecLayers)
		{
			auto layerObj = layersArray.AddObject();
			layer.Save(layerObj);
		}
	}

	void TileMap::Load(const rapidjson::Value &data)
	{
		std::vector<TileLayer> newLayers;		

		auto size = TileCoord_t{ 
			static_cast<uint8_t>(data["width"].GetInt()), 
			static_cast<uint8_t>(data["height"].GetInt()) 
		};

		CheckSize(size);

		auto layersData = data["layers"].GetArray();
		auto numLayers = layersData.Size();

		if (numLayers == 0)
		{
			throw std::invalid_argument("[TileMap::Load] numLayers must be >= 0");
		}
		
		newLayers.reserve(numLayers);
		for (unsigned i = 0; i < numLayers; ++i)
		{
			newLayers.emplace_back(size);
			newLayers[i].Load(layersData[i]);
		}

		std::swap(m_vecLayers, newLayers);
	}
}
