// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <functional>
#include <optional>

#include "FileSystem.h"

namespace dcclite::DirectoryMonitor
{
	enum MonitorActions
	{
		MONITOR_ACTION_FILE_CREATE = 0x01,
		MONITOR_ACTION_FILE_DELETE = 0x02,
		MONITOR_ACTION_FILE_MODIFY = 0x04,
		MONITOR_ACTION_FILE_RENAME_OLD_NAME = 0x08,
		MONITOR_ACTION_FILE_RENAME_NEW_NAME = 0x10
	};

	typedef std::function<void(const dcclite::fs::path &path, std::string fileName, const uint32_t action)> Callback_t;

	void Watch(const dcclite::fs::path &path, Callback_t callback, const uint32_t action);
	bool Unwatch(const dcclite::fs::path &path);

	namespace detail
	{
		std::optional<bool> IsThreadWaiting(const dcclite::fs::path &path);
	}	
}
