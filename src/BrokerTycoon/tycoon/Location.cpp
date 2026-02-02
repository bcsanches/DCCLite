// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Location.h"

#include <fmt/format.h>

#include <rapidjson/document.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>

#include "Industry.h"

namespace dcclite::broker::tycoon
{
	const char *Location::TYPE_NAME = "dcclite::broker::tycoon::Location";

	Location::Location(RName name, TycoonService &tycoon, const rapidjson::Value &params):
		FolderObject{name}
	{
		auto industriesArray = dcclite::json::TryGetValue(params, "industries");
		if (!industriesArray)
			return;

		if (!industriesArray->IsArray())
			throw std::runtime_error(fmt::format("[Tycoon::Location::{}] [constructor] error: invalid industries definition, expected array", this->GetName()));

		for (const auto &industryValue : industriesArray->GetArray())
		{
			auto industryName = dcclite::json::TryGetString(industryValue, "name");
			if (!industryName)
			{
				throw std::runtime_error(fmt::format("[Tycoon::Location::{}] [constructor] error: industry without a name", this->GetName()));
			}

			this->AddChild(std::make_unique<Industry>(RName{ industryName.value() }, tycoon, industryValue));
		}
	}

	IObject *Location::AddChild(std::unique_ptr<Object> obj)
	{
		//only Industry objects are allowed as children
		if (!dynamic_cast<Industry *>(obj.get()))
		{
			throw std::invalid_argument(
				fmt::format(
					"[Tycoon::Location::{}] [AddChild] error: only Industry objects are allowed as children, got '{}'",
					this->GetName(),
					obj->GetTypeName()
				)
			);
		}

		return FolderObject::AddChild(std::move(obj));
	}
}

