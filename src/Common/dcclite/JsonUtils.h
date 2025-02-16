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

namespace dcclite::json
{
	bool TryGetDefaultBool(const rapidjson::Value &data, const char *fieldName, const bool defaultValue);

	int TryGetDefaultInt(const rapidjson::Value &data, const char *fieldName, const int defaultValue);

	const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context);

	const char *GetString(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	const char *TryGetDefaultString(const rapidjson::Value &data, const char *fieldName, const char *defaultValue);


	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);

	const rapidjson::Value::ConstArray GetArray(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
}
