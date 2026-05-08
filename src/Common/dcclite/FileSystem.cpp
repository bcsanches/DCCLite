// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FileSystem.h"

#include "Log.h"

#include <fstream>

namespace dcclite::FileSystem
{
	bool CreateFilePath(const dcclite::fs::path &filePath)
	{
		auto path = filePath;
		path.remove_filename();

		std::error_code ec;
		dcclite::fs::create_directories(path, ec);
		if (ec)
		{
			dcclite::Log::Error("[FileSystem::CreateFilePath] {} Cannot create app path for file: {}", filePath.string(), ec.message());
			return false;
		}

		return true;
	}

	bool SafeStoreText(const dcclite::fs::path &filePath, const char *FileExtension, const char *content)
	{
		auto finalPath = filePath;
		finalPath.concat(".tmp");

		dcclite::Log::Info("[FileSystem::SafeStoreText] Generating file: {}", filePath.string());

		if (!dcclite::FileSystem::CreateFilePath(finalPath))
		{
			dcclite::Log::Error("[FileSystem::SafeStoreText] Cannot create path {} for storing text", filePath.string());

			return false;
		}

		{
			std::ofstream newStateFile(finalPath, std::ios_base::trunc);

			newStateFile << content;
		}

		auto newName{ finalPath };
		newName.replace_extension(".json");
		dcclite::fs::rename(finalPath, newName);

		dcclite::Log::Info("[FileSystem::SafeStoreText] Text file stored at: {}", newName.string());

		return true;
	}
}
