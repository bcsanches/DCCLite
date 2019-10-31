// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Project.h"

#include <fstream>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "FmtUtils.h"
#include "GuidUtils.h"
#include "PathUtils.h"

#include "Log.h"


dcclite::fs::path Project::GetAppFilePath(const std::string_view fileName) const
{
	auto cacheFilePath = dcclite::PathUtils::GetAppFolder();

	cacheFilePath.append(m_strName);
	cacheFilePath.append(fileName);

	return cacheFilePath;
}

dcclite::Guid Project::GetFileToken(const std::string_view fileName) const
{	
	dcclite::Sha1 currentFileHash;
	currentFileHash.ComputeForFile(this->GetFilePath(fileName));

	//dcclite::Log::Trace("hash {} -> {}", filePath.string(), currentFileHash.ToString());

	dcclite::Guid token;
	dcclite::Sha1 storedHash;

	std::string stateFileName(fileName);
	stateFileName.append(".state");

	auto stateFilePath = this->GetAppFilePath(stateFileName);

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
				dcclite::Log::Error("project state file does not contain token");
				goto SKIP_LOAD;
			}

			//read tokenStr
			if (!dcclite::TryGuidLoadFromString(token, tokenData->value.GetString()))
			{
				dcclite::Log::Error("project error parsing stored token");
				goto SKIP_LOAD;
			}	

			auto hashData = stateData.FindMember("sha1");
			if ((hashData == stateData.MemberEnd()) || (!hashData->value.IsString()))
			{
				dcclite::Log::Error("Project state file does not contain hash");

				goto SKIP_LOAD;
			}
			
			//read hash
			if (!storedHash.TryLoadFromString(hashData->value.GetString()))
			{
				dcclite::Log::Error("Project error parsing hash");
				goto SKIP_LOAD;
			}			
		}
		else
		{
			dcclite::Log::Info("Project state file for {} not found", fileName);
		}

SKIP_LOAD:
		if (storedHash != currentFileHash)
		{
			dcclite::Log::Info("Project config file {} modified", fileName);

			token = dcclite::GuidCreate();
			
			dcclite::fs::path filePath = stateFilePath;
			filePath.remove_filename();

			std::error_code ec;
			dcclite::fs::create_directories(filePath, ec);

			if (ec)
			{
				dcclite::Log::Error("Cannot create app path for storing state for {}, system error: {}", fileName, ec.message());				
			}
			else
			{
				std::ofstream newStateFile(stateFilePath, std::ios_base::trunc);

				JsonCreator::StringWriter responseWriter;
				{
					auto object = JsonCreator::MakeObject(responseWriter);

					object.AddStringValue("token", fmt::format("{}", token));
					object.AddStringValue("sha1", currentFileHash.ToString());
				}
							
				newStateFile << responseWriter.GetString();

				dcclite::Log::Info("Stored {} state data on {}", fileName, stateFilePath.string());
			}
		}
	}	

	return token;
}

