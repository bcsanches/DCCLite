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

#include <string>
#include <string_view>

namespace dcclite
{
	class Guid;

	extern Guid GuidCreate();

	extern std::string GuidToString(const dcclite::Guid &g);

	extern bool TryGuidLoadFromString(Guid &dest, std::string_view str);
}
