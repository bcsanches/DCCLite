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

#include "FmtUtils.h"
#include "Device.h"
#include "GuidUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "RName.h"

#include "../sys/Project.h"

namespace dcclite::broker::StorageManager
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

	void SaveState(const Device &device, const Project &project)
	{
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
				return;
		}
		
		auto path = project.GetAppFilePath(fmt::format("{}.state.tmp", device.GetName().GetData()));

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
	}

	dcclite::Guid GetFileToken(const std::string_view fileName, const Project &project)
	{
		dcclite::Sha1 currentFileHash;
		currentFileHash.ComputeForFile(project.GetFilePath(fileName));

		dcclite::Log::Trace("[StorageManager::GetFileToken] {} hash is {}", fileName, currentFileHash.ToString());

		dcclite::Guid token;
		dcclite::Sha1 storedHash;

		dcclite::fs::path stateFileName(fileName);
		stateFileName.replace_extension(".token.json");				

		auto stateFilePath = project.GetAppFilePath(stateFileName.string());

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
