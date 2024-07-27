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

	const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = data.FindMember(fieldName);

		if (field == data.MemberEnd())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found on {}", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found", fieldName));
		}

		return field->value;
	}

	const char *TryGetDefaultString(const rapidjson::Value &data, const char *fieldName, const char *defaultValue)
	{
		const auto &field = data.FindMember(fieldName);

		return ((field == data.MemberEnd()) ? defaultValue : field->value.GetString());
	}

	const char *GetString(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);
		
		if (!field.IsString())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be a string", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be a string", fieldName));
		}

		return field.GetString();
	}

	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);

		if (!field.IsInt())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be an int", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be an int", fieldName));
		}

		return field.GetInt();

	}

	const rapidjson::Value::ConstArray GetArray(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);

		if (!field.IsArray())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetArray] Required field {} on {} must be an array", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be an array", fieldName));
		}

		return field.GetArray();
	}
}
