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

#include <functional>

#include <FileSystem.h>

namespace FileWatcher
{
	enum Flags
	{
		FW_MODIFIED = 0x01,		
	};

	struct Event
	{
		std::string m_strPath;
		std::string m_strFileName;

		uint32_t	m_fFlags;
	};

	typedef std::function<void(const Event &data)> Callback_t;

	bool TryWatchFile(const dcclite::fs::path &fileName, const uint32_t flags, const Callback_t &callback);

	void UnwatchFile(const dcclite::fs::path &fileName);
}
