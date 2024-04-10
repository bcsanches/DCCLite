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

#include "LitePanelLibDefs.h"

#include "MapObject.h"

namespace LitePanel
{	
	class TileLayer
	{
		public:
			TileLayer(const TileCoord_t size);
			TileLayer(TileLayer &&other) noexcept;

			TileLayer() = delete;
			TileLayer(const TileLayer &) = delete;

			void RegisterObject(std::unique_ptr<MapObject> object);
			std::unique_ptr<MapObject> UnregisterObject(const MapObject& obj);			

			const MapObject *TryGetMapObject(const TileCoord_t pos) const;

			const TileCoord_t &GetSize() const noexcept { return m_tSize; }

			void Save(JsonOutputStream_t& stream) const;
			void Load(const rapidjson::Value& data);

		private:
			size_t GetIndex(const TileCoord_t &position) const;

		private:
			//It is redundant to save the size on every layer, but makes life easier
			TileCoord_t m_tSize;

			std::vector<std::unique_ptr<MapObject>> m_vecMap;
	};

	class ITileMapListener
	{
		public:
			virtual void TileMap_OnStateChanged() = 0;
	};

	class TileMap
	{
		public:
			TileMap(const TileCoord_t size, const unsigned numLayers = 1);
			TileMap(const rapidjson::Value& data);

			TileMap(TileMap &&) = default;

			const TileCoord_t &GetSize() const noexcept { return m_vecLayers[0].GetSize(); }

			void RegisterObject(std::unique_ptr<MapObject> object, const uint8_t layer);	
			std::unique_ptr<MapObject> UnregisterObject(const MapObject &obj, const uint8_t layer);
			std::unique_ptr<MapObject> TryUnregisterObject(const TileCoord_t &position, const uint8_t layer);

			bool IsTileOccupied(const TileCoord_t &position, const uint8_t layer) const;

			inline uint8_t GetNumLayers() const
			{
				return static_cast<uint8_t>(m_vecLayers.size());
			}

			inline const TileLayer *GetLayers() const
			{
				return &m_vecLayers[0];
			}

			[[nodiscard]] bool IsInside(const TileCoord_t &position) const noexcept
			{
				auto &size = this->GetSize();

				if (position.m_tX >= size.m_tX)
					return false;

				if (position.m_tY >= size.m_tY)
					return false;

				return true;
			}

			[[nodiscard]] bool IsInside(const IntPoint_t &position) const noexcept
			{
				auto &size = this->GetSize();

				if (position.m_tX >= size.m_tX)
					return false;

				if (position.m_tY >= size.m_tY)
					return false;

				return true;
			}

			const MapObject *TryGetMapObject(const TileCoord_t pos, const uint8_t layer) const;

			void AddListener(ITileMapListener *listener);
			void RemoveListener(ITileMapListener *listener);

			void Save(JsonOutputStream_t& stream) const;
			void Load(const rapidjson::Value &data);

		private:
			void StateChanged();

		private:			
			std::vector<TileLayer> m_vecLayers;

			std::vector<ITileMapListener *> m_vecListeners;
	};
}
