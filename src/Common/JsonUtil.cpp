// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "JsonUtils.h"

#include <stdexcept>

#include <fmt/format.h>

namespace dcclite::json
{
	bool TryGetDefaultBool(const rapidjson::Value &data, const char *fieldName, const bool defaultValue)
	{
		const auto &field = data.FindMember(fieldName);

		return ((field == data.MemberEnd()) ? defaultValue : field->value.GetBool());
	}

	int TryGetDefaultInt(const rapidjson::Value &data, const char *fieldName, const int defaultValue)
	{
		const auto &field = data.FindMember(fieldName);

		return ((field == data.MemberEnd()) ? defaultValue : field->value.GetInt());
	}

	static inline const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = data.FindMember(fieldName);

		if (field == data.MemberEnd())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found on {}", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found", fieldName));
		}

		return field;
	}

	const char *GetString(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetMember(data, fieldName, context);
		
		if (!field->value.IsString())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be a string", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be a string", fieldName));
		}

		return field->value.GetString();
	}

	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context = nullptr)
	{
		const auto &field = GetMember(data, fieldName, context);

		if (!field->value.IsInt())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be an int", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be an int", fieldName));
		}

		return field->value.GetInt();

	}

	const rapidjson::Value::ConstArray GetArray(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetMember(data, fieldName, context);

		if (!field->value.IsArray())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetArray] Required field {} on {} must be an array", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be an array", fieldName));
		}

		return field->value.GetArray();
	}
}
