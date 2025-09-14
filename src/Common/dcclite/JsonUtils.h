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

#include <optional>
#include <string_view>

namespace dcclite::json
{
	const rapidjson::Value *TryGetValue(const rapidjson::Value &data, const char *fieldName);

	bool TryGetDefaultBool(const rapidjson::Value &data, const char *fieldName, const bool defaultValue);

	std::optional<int> TryGetInt(const rapidjson::Value &data, const char *fieldName);
	int TryGetDefaultInt(const rapidjson::Value &data, const char *fieldName, const int defaultValue);

	const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context);

	std::string_view GetString(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	std::string_view TryGetDefaultString(const rapidjson::Value &data, const char *fieldName, std::string_view defaultValue);
	std::optional<std::string_view> TryGetString(const rapidjson::Value &data, const char* fieldName);	

	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	
	const rapidjson::Value::ConstArray GetArray(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
}
