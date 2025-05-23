// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "StorageManager.h"

#include <fstream>

#include <fmt/format.h>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/Guid.h>
#include <dcclite/Log.h>
#include <dcclite/RName.h>

#include "Device.h"
#include "OutputDecoder.h"

#include "sys/Project.h"

//Win32 headers leak...
#undef GetObject

namespace dcclite::broker::exec::dcc::StorageManager
{	
	static bool CreateFilePath(const dcclite::fs::path &filePath)
	{		
		auto path = filePath;
		path.remove_filename();

		std::error_code ec;
		dcclite::fs::create_directories(path, ec);

		if (ec)
		{
			dcclite::Log::Error("[StorageManager::GetFileToken] {} Cannot create app path for storing state, system error: {}", filePath.string(), ec.message());

			return false;
		}

		return true;
	}

	static dcclite::fs::path GenerateBaseDeviceStateFileName(RName deviceName)
	{
		return sys::Project::GetAppFilePath(fmt::format("{}.state", deviceName.GetData()));
	}

	DecodersMap_t LoadState(RName deviceName, const dcclite::Guid expectedToken)
	{
		BenchmarkLogger benchmark{ "StorageManager::LoadState", deviceName.GetData() };

		auto path = GenerateBaseDeviceStateFileName(deviceName);
		path.concat(".json");

		std::ifstream stateFile(path);

		if (!stateFile)
		{
			dcclite::Log::Warn("[StorageManager::LoadState] [{}] Failed to open state file: {}", deviceName.GetData(), path.string());

			return {};
		}

		dcclite::Log::Info("[StorageManager::LoadState] [{}] Opened {}, starting parser", deviceName.GetData(), path.string());

		rapidjson::IStreamWrapper isw(stateFile);
		rapidjson::Document data;
		if (data.ParseStream(isw).HasParseError())
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] Parser error for {}", deviceName.GetData(), path.string());
			
			return {};
		}

		//read token first, because if it fails, hash is already null			
		auto tokenData = data.FindMember("token");
		if ((tokenData == data.MemberEnd()) || (!tokenData->value.IsString()))
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] State file does not contain token: {}", deviceName.GetData(), path.string());
			
			return {};
		}

		dcclite::Guid token;

		//read tokenStr
		if (!dcclite::TryGuidLoadFromString(token, tokenData->value.GetString()))
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] error parsing stored token: {}", deviceName.GetData(), path.string());
			
			return {};
		}

		dcclite::Log::Trace("[StorageManager::LoadState] [{}] config token on state file is {}", deviceName.GetData(), token);

		//
		//we cannot compare against device current token, because it may havent loaded it yet, so it must provide a token to us
		if (token != expectedToken)
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] config token on file is invalid, ignoring config: {}", deviceName.GetData(), path.string());

			return {};
		}

		auto decodersData = data.FindMember("decoders");
		if (decodersData == data.MemberEnd())
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] State file does not contain decoders data: {}", deviceName.GetData(), path.string());

			return {};
		}

		if (!decodersData->value.IsObject())
		{
			dcclite::Log::Error("[StorageManager::LoadState] [{}] State file decoders data is not an object: {}", deviceName.GetData(), path.string());

			return {};
		}

		//
		//got a valid config, load states
		DecodersMap_t decodersState;

		for (auto &it : decodersData->value.GetObject())
		{
			RName decoderName{ it.name.GetString() };

			if (!it.value.IsBool())
			{
				dcclite::Log::Error("[StorageManager::LoadState] [{}] State data for decoder {} is not a bool: {}", deviceName.GetData(), it.name.GetString(), path.string());
				continue;
			}

			decodersState[decoderName] = it.value.GetBool() ? DecoderStates::ACTIVE : DecoderStates::INACTIVE;
		}

		dcclite::Log::Info("[StorageManager::LoadState][{}] State file loaded.", deviceName.GetData());
		return decodersState;
	}

	void SaveState(const Device &device)
	{
		BenchmarkLogger benchmark{ "StorageManager::SaveState", device.GetNameData()};

		bool dataStored = false;

		JsonCreator::StringWriter responseWriter;
		{
			auto object = JsonCreator::MakeObject(responseWriter);

			object.AddStringValue("token", fmt::format("{}", device.GetConfigToken()));
			{
				auto decoders = object.AddObject("decoders");

				device.ConstVisitDecoders([&decoders, &dataStored](const IObject &obj) 
				{
					if (auto decoder = dynamic_cast<const OutputDecoder *>(&obj))
					{
						if (decoder->IgnoreSavedState())
							return true;

						dataStored = true;						
						decoders.AddBool(decoder->GetNameData(), decoder->GetRequestedState() == dcclite::DecoderStates::ACTIVE);
					}

					return true;
				});
			}

			if (!dataStored)
			{
				dcclite::Log::Info("[StorageManager::SaveState][{}] No data to save, aborting", device.GetNameData());
				return;
			}
				
		}		
		
		auto path = GenerateBaseDeviceStateFileName(device.GetName());
		path.concat(".tmp");

		dcclite::Log::Info("[StorageManager::SaveState][{}] Generating state file: {}", device.GetNameData(), path.string());

		if (!CreateFilePath(path))
		{
			dcclite::Log::Error("[StorageManager::SaveState] Cannot create path {} for storing {} device data", path.string(), device.GetNameData());

			return;
		}

		{
			std::ofstream newStateFile(path, std::ios_base::trunc);

			newStateFile << responseWriter.GetString();
		}

		auto newName{ path };
		newName.replace_extension(".json");
		dcclite::fs::rename(path, newName);

		dcclite::Log::Info("[StorageManager::SaveState][{}] State file stored at: {}", device.GetNameData(), newName.string());
	}

	dcclite::Guid GetFileToken(const std::string_view fileName)
	{
		BenchmarkLogger benchmark{ "StorageManager::GetFileToken", fileName };

		dcclite::Sha1 currentFileHash;

		{
			BenchmarkLogger benchmark{ "StorageManager::GetFileToken::Hash", fileName };

			currentFileHash.ComputeForFile(sys::Project::GetFilePath(fileName));
		}

		dcclite::Log::Trace("[StorageManager::GetFileToken] {} hash is {}", fileName, currentFileHash.ToString());

		dcclite::Guid token;
		dcclite::Sha1 storedHash;

		dcclite::fs::path stateFileName(fileName);
		stateFileName.replace_extension(".token.json");				

		auto stateFilePath = sys::Project::GetAppFilePath(stateFileName.string());

		{
			std::ifstream stateFile(stateFilePath);
			if (stateFile)
			{
				using namespace rapidjson;

				IStreamWrapper isw(stateFile);
				Document stateData;
				stateData.ParseStream(isw);

				//read token first, because if it fails, hash is already null			
				auto tokenData = stateData.FindMember("token");
				if ((tokenData == stateData.MemberEnd()) || (!tokenData->value.IsString()))
				{
					dcclite::Log::Error("[StorageManager::GetFileToken] {} state file does not contain token", stateFilePath.string());
					goto SKIP_LOAD;
				}

				//read tokenStr
				if (!dcclite::TryGuidLoadFromString(token, tokenData->value.GetString()))
				{
					dcclite::Log::Error("[StorageManager::GetFileToken] {} error parsing stored token", stateFilePath.string());
					goto SKIP_LOAD;
				}

				auto hashData = stateData.FindMember("sha1");
				if ((hashData == stateData.MemberEnd()) || (!hashData->value.IsString()))
				{
					dcclite::Log::Error("[StorageManager::GetFileToken] {} state file does not contain hash", stateFilePath.string());

					goto SKIP_LOAD;
				}

				//read hash
				if (!storedHash.TryLoadFromString(hashData->value.GetString()))
				{
					dcclite::Log::Error("[StorageManager::GetFileToken] {} error parsing hash", stateFilePath.string());
					goto SKIP_LOAD;
				}
			}
			else
			{
				dcclite::Log::Info("[StorageManager::GetFileToken] {} state file not found", fileName);
			}

		SKIP_LOAD:
			if (storedHash != currentFileHash)
			{
				dcclite::Log::Info("[StorageManager::GetFileToken] {} config file modified", fileName);

				token = dcclite::GuidCreate();

				if(CreateFilePath(stateFilePath))
				{
					std::ofstream newStateFile(stateFilePath, std::ios_base::trunc);

					JsonCreator::StringWriter responseWriter;
					{
						auto object = JsonCreator::MakeObject(responseWriter);

						object.AddStringValue("token", fmt::format("{}", token));
						object.AddStringValue("sha1", currentFileHash.ToString());
					}

					newStateFile << responseWriter.GetString();

					dcclite::Log::Info("[StorageManager::GetFileToken] {} state data on {}", fileName, stateFilePath.string());
				}
			}
			else
			{
				dcclite::Log::Info("[StorageManager::GetFileToken] {} stored hash on state match {}", fileName, storedHash.ToString());
			}
		}

		return token;
	}
}
