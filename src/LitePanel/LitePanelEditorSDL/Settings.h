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

#include <optional>
#include <vector>

#include "FileSystem.h"

namespace dcclite::panel_editor::Settings
{
	[[nodiscard]] std::optional<dcclite::fs::path> GetLastProjectPath();

	void AddRecentProject(dcclite::fs::path path);

	//
	//This remains valid until you call any settings function
	[[nodiscard]] const std::vector< dcclite::fs::path> &GetRecentFiles();
}
