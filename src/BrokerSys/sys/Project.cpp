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

#include <dcclite/PathUtils.h>

namespace dcclite::broker::sys::Project
{
	static dcclite::fs::path g_pthRoot;
	static std::string g_strName;

	void SetWorkingDir(dcclite::fs::path path)
	{
		g_pthRoot = std::move(path);
	}

	dcclite::fs::path GetFilePath(const std::string_view fileName)
	{
		dcclite::fs::path path(g_pthRoot);

		path.append(fileName);

		return path.string();
	}

	void SetName(std::string_view name)
	{
		g_strName = name;
	}

	const std::string &GetName() noexcept
	{
		return g_strName;
	}

	const dcclite::fs::path &GetRoot() noexcept
	{
		return g_pthRoot;
	}

	dcclite::fs::path GetAppFilePath(const std::string_view fileName)
	{
		auto cacheFilePath = dcclite::PathUtils::GetAppFolder();

		cacheFilePath.append(g_strName);
		cacheFilePath.append(fileName);

		return cacheFilePath;
	}
}
