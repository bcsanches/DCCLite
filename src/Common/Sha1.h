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

#include <cstring>

#include "FileSystem.h"

namespace dcclite
{
	struct Sha1
	{		
		unsigned char mData[20];

		Sha1();

		std::string ToString() const;

		void ComputeForFile(const fs::path &fileName);
		bool TryLoadFromString(std::string_view str);

		inline bool operator!=(const Sha1 &rhs) const
		{
			return memcmp(mData, rhs.mData, sizeof(mData)) != 0;
		}
	};		
} //end of namespace dcclite
	