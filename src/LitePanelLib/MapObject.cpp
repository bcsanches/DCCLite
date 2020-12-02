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

#include "MapObjectFactory.h"

namespace LitePanel
{
	MapObject::MapObject(const TileCoord_t &position):
		m_tPosition{position}
	{
		//empty
	}	

	MapObject::MapObject(const rapidjson::Value &params):
		m_tPosition{params["x"].GetInt(), params["y"].GetInt() }
	{
		//empty
	}

	void MapObject::Save(JsonOutputStream_t &stream) const noexcept
	{
		stream.AddIntValue("x", m_tPosition.m_tX);
		stream.AddIntValue("y", m_tPosition.m_tY);
		stream.AddStringValue("classname", this->GetTypeName());

		this->OnSave(stream);
	}

	void MapObject::OnSave(JsonOutputStream_t &stream) const noexcept
	{
		//empty
	}

	std::unique_ptr<MapObject> MapObject::Create(const rapidjson::Value &params)
	{				
		return MapObjectFactory::Create(params["classname"].GetString(), params);
	}
}
