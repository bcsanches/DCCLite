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

#include <dcclite/FileSystem.h>
#include <dcclite/Sha1.h>

namespace dcclite::broker
{
	namespace Project
	{
		void SetWorkingDir(dcclite::fs::path path);

		dcclite::fs::path GetFilePath(const std::string_view fileName);
		dcclite::fs::path GetAppFilePath(const std::string_view fileName);

		void SetName(std::string_view name);			

		const std::string &GetName() noexcept;			

		/// <summary>
		/// 
		/// </summary>
		/// <returns>Returns the path to the root used for the current project</returns>
		const dcclite::fs::path &GetRoot() noexcept;
	}
}
