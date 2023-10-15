// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "MapObjectFactory.h"

#include <functional>

#include "fmt/format.h"

#include "RailObject.h"

namespace LitePanel::MapObjectFactory
{
	struct Factory
	{
		const char *m_pszName;

		std::function<std::unique_ptr<MapObject>(const rapidjson::Value &params)> m_pfnProc;
	};

	static Factory g_stCreators[] =
	{
		{LitePanel::SimpleRailObject::TYPE_NAME, [](const rapidjson::Value &params) {return std::make_unique< SimpleRailObject>(params); }},
		{nullptr, nullptr}
	};

	std::unique_ptr<MapObject> Create(std::string_view name, const rapidjson::Value &params)
	{
		for (auto f = g_stCreators; f->m_pszName; ++f)
		{
			if (name.compare(f->m_pszName) == 0)
				return f->m_pfnProc(params);			
		}

		throw std::invalid_argument(fmt::format("[LitePanel::MapObjectFactory::Create] Unknown type {}", name));
	}
}
