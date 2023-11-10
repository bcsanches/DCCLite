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

#include <rapidjson/document.h>

#include "LitePanelLibDefs.h"

namespace LitePanel
{	
	enum class ObjectAngles
	{
		EAST = 0,
		NORTHEAST = 45,
		NORTH = 90,
		NORTHWEST = 135,
		WEST = 180,
		SOUTHWEST = 225,
		SOUTH = 270,
		SOUTHEAST = 315
	};

	class MapObject
	{
		public:
			MapObject(const TileCoord_t &position);
			MapObject(const rapidjson::Value &params);

			virtual ~MapObject() = default;

			inline const TileCoord_t &GetPosition() const noexcept
			{
				return m_tPosition;
			}

			void Save(JsonOutputStream_t& stream) const noexcept;

			virtual bool IsPersistent() const noexcept
			{
				return true;
			}

			virtual const char* GetTypeName() const noexcept
			{
				return TYPE_NAME;
			}

			static constexpr char* TYPE_NAME = "MapObject";

			static std::unique_ptr<MapObject> Create(const rapidjson::Value& defs);

		protected:
			virtual void OnSave(JsonOutputStream_t& stream) const noexcept;

		private:
			friend class Panel;			

			void SetPosition(const TileCoord_t& position)
			{
				m_tPosition = position;
			}

		private:
			TileCoord_t m_tPosition;
	};
}
