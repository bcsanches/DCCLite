// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "PathUtils.h"

static std::string g_strAppName;

void dcclite::PathUtils::SetAppName(std::string_view name)
{
	g_strAppName = name;
}

#ifdef WIN32

#include "Shlobj.h"

std::filesystem::path dcclite::PathUtils::GetAppFolder()
{
	PWSTR pFolderPath = nullptr;

	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pFolderPath);
	if (hr != S_OK)
	{
		throw std::runtime_error("error: dcclite::GetAppFolder cannot get CSIDL_LOCAL_APPDATA");
	}

	std::filesystem::path result(pFolderPath);
	result.append("dcclite");
	result.append(g_strAppName);

	CoTaskMemFree(pFolderPath);

	return result;
}

#else

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

std::filesystem::path dcclite::PathUtils::GetAppFolder()
{
	const char* homedir;

	if ((homedir = getenv("HOME")) == nullptr) 
	{		
		throw std::runtime_error("error: dcclite::GetAppFolder failed to call getenv(\"HOME\")");
	}

	std::filesystem::path result(homedir);
	result.append(".dcclite");
	result.append(g_strAppName);	

	return result;
}

#endif