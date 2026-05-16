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

#include "FileSystem.h"

namespace dcclite::json
{
	const rapidjson::Value *TryGetValue(const rapidjson::Value &data, const char *fieldName);
	const rapidjson::Value *TryGetObject(const rapidjson::Value &data, const char *fieldName);

	bool GetBool(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	bool TryGetDefaultBool(const rapidjson::Value &data, const char *fieldName, const bool defaultValue);

	std::optional<int> TryGetInt(const rapidjson::Value &data, const char *fieldName);
	int TryGetDefaultInt(const rapidjson::Value &data, const char *fieldName, const int defaultValue);

	const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context);
	const rapidjson::Value &GetObject(const rapidjson::Value &data, const char *fieldName, const char *context);

	std::string_view GetString(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	std::string_view TryGetDefaultString(const rapidjson::Value &data, const char *fieldName, std::string_view defaultValue);
	std::optional<std::string_view> TryGetString(const rapidjson::Value &data, const char* fieldName);	

	int64_t GetInt64(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);

	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	int GetRangedInt(const rapidjson::Value &data, const char *fieldName, int min, int max, const char *context = nullptr);
	float GetFloat(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);
	
	const rapidjson::Value::ConstArray GetArray(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr);

	inline std::string_view MakeStringView(const rapidjson::Value &value)
	{
		return std::string_view{ value.GetString(), value.GetStringLength() };
	}

	class FileDocument
	{
		public:
			[[nodiscard]] bool Load(const dcclite::fs::path &path);

			[[nodiscard]] inline bool IsObject() const noexcept
			{
				return m_docJson.IsObject();
			}

			[[nodiscard]] rapidjson::Document::ConstObject GetObject() const noexcept
			{
				return m_docJson.GetObject();
			}

			[[nodiscard]] inline bool IsArray() const noexcept
			{
				return m_docJson.IsArray();
			}

			[[nodiscard]] rapidjson::Document::ConstArray GetArray() const noexcept
			{
				return m_docJson.GetArray();
			}

		private:
			rapidjson::Document m_docJson;
	};
}
