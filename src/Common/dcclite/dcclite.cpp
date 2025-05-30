// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.
#include "dcclite.h"

#include "Log.h"
#include "PathUtils.h"
#include "Util.h"

namespace dcclite
{
	void Init(std::string_view appFolderName, const char *logFile)
	{
		SetMainThread();

		Log::detail::Init(logFile);

		PathUtils::detail::Init(appFolderName);		
	}

	void Finalize()
	{
		PathUtils::detail::Finalize();

		Log::detail::Finalize();

		ClearMainThread();
	}
}
