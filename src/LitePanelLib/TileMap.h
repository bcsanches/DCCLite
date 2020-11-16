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

namespace LitePanel
{
	class MapObject;

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

			void AddListener(ITileMapListener *listener);
			void RemoveListener(ITileMapListener *listener);

			void Save(JsonOutputStream_t& stream) const;

		private:
			void StateChanged();

		private:			
			std::vector<TileLayer> m_vecLayers;

			std::vector< ITileMapListener *> m_vecListeners;
	};
}
