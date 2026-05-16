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

#include <fstream>
#include <stdexcept>

#include <fmt/format.h>

#include <rapidjson/istreamwrapper.h>

#include <magic_enum/magic_enum.hpp>

#include "Log.h"

namespace dcclite::json
{
	const rapidjson::Value *TryGetValue(const rapidjson::Value &data, const char *fieldName)
	{
		const auto &field = data.FindMember(fieldName);
		if (field == data.MemberEnd())
			return nullptr;

		return &field->value;
	}

	const rapidjson::Value *TryGetObject(const rapidjson::Value &data, const char *fieldName)
	{
		auto field = TryGetValue(data, fieldName);

		if (!field)
			return nullptr;

		if (!field->IsObject())
			return nullptr;

		return field;		
	}

	bool GetBool(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);
		if (!field.IsBool())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetBool] Required field {} on {} must be a boolean", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetBool] Required field {} must be a boolean", fieldName));
		}

		return field.GetBool();
	}

	bool TryGetDefaultBool(const rapidjson::Value &data, const char *fieldName, const bool defaultValue)
	{
		auto field = TryGetValue(data, fieldName);

		return ((field == nullptr) ? defaultValue : field->GetBool());
	}

	std::optional<int> TryGetInt(const rapidjson::Value &data, const char *fieldName)
	{
		auto field = TryGetValue(data, fieldName);

		if (field)
			return field->GetInt();
		else
			return std::nullopt;
	}

	int TryGetDefaultInt(const rapidjson::Value &data, const char *fieldName, const int defaultValue)
	{
		auto field = TryGetValue(data, fieldName);

		return (field == nullptr) ? defaultValue : field->GetInt();
	}

	const rapidjson::Value &GetValue(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		auto field = TryGetValue(data, fieldName);

		if (!field)
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found on {}", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetMember] Required field {} not found", fieldName));
		}

		return *field;
	}

	const rapidjson::Value &GetObject(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		auto &field = GetValue(data, fieldName, context);

		if (!field.IsObject())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetObject] Required field {} on {} must be an object", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetObject] Required field {} must be an object", fieldName));
		}

		return field;
	}

	std::optional<std::string_view> TryGetString(const rapidjson::Value &data, const char *fieldName)
	{
		auto field = TryGetValue(data, fieldName);
		if (!field)
			return std::nullopt;

		return std::string_view{ field->GetString(), field->GetStringLength() };
	}

	std::string_view TryGetDefaultString(const rapidjson::Value &data, const char *fieldName, std::string_view defaultValue)
	{
		auto str = TryGetString(data, fieldName);		

		return str.has_value() ? *str : defaultValue;		
	}

	std::string_view GetString(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		auto str = TryGetString(data, fieldName);

		[[unlikely]]
		if(!str)		
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be a string", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be a string", fieldName));
		}

		return str.value();
	}

	int64_t GetInt64(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);

		if (!field.IsInt64())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetInt64] Required field {} on {} must be an int", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetInt64] Required field {} must be an int", fieldName));
		}

		return field.GetInt64();
	}

	int GetInt(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);

		if (!field.IsInt())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetInt] Required field {} on {} must be an int", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetInt] Required field {} must be an int", fieldName));
		}

		return field.GetInt();

	}

	int GetRangedInt(const rapidjson::Value &data, const char *fieldName, int min, int max, const char *context)
	{
		auto value = GetInt(data, fieldName, context);

		if(value < min)
			throw std::runtime_error(fmt::format("[dcclite::json::GetInt] Required integer {} - {} must be greater than {}", fieldName, value, min));

		if (value > max)
			throw std::runtime_error(fmt::format("[dcclite::json::GetInt] Required integer {} - {} must be less than {}", fieldName, value, min));

		return value;
	}

	float GetFloat(const rapidjson::Value &data, const char *fieldName, const char *context)
	{
		const auto &field = GetValue(data, fieldName, context);

		if (field.IsInt())
			return static_cast<float>(field.GetInt());

		if (!field.IsFloat())
		{
			if (context)
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} on {} must be a float", fieldName, context));
			else
				throw std::runtime_error(fmt::format("[dcclite::json::GetString] Required field {} must be a float", fieldName));
		}

		return field.GetFloat();
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
	
	[[nodiscard]] bool FileDocument::Load(const dcclite::fs::path &path)
	{
		std::ifstream configFile(path);

		if (!configFile)
		{
			dcclite::Log::Warn("[json::FileDocument::Load] cannot find {}", path.string());

			return false;
		}

		rapidjson::IStreamWrapper isw(configFile);

		m_docJson.ParseStream(isw);
		if (m_docJson.HasParseError())
		{
			throw std::runtime_error(
				fmt::format(
					"[json::FileDocument::Load] error parsing JSON file {}: {}",
					path.string(),
					magic_enum::enum_name(m_docJson.GetParseError())
				)
			);
		}

		return true;
	}	
}
