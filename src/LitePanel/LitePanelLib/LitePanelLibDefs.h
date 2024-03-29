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

#include "Point.h"

#include "JsonCreator/Object.h"
#include "JsonCreator/StringWriter.h"

#include <rapidjson/document.h>

namespace LitePanel
{
	class EditCmd;
	class MapObject;
	class Panel;
	class RailObject;

	constexpr auto DEFAULT_TILE_SIZE = 32;

	typedef Point<uint16_t> TileCoord_t;

	typedef JsonCreator::Object<JsonCreator::StringWriter> JsonOutputStream_t;
}
