// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <functional>
#include <memory>

namespace dcclite::broker::tycoon
{
	class TycoonService;
}

extern std::unique_ptr<dcclite::broker::tycoon::TycoonService> LoadTycoon(const char *json, bool deleteExistingState = false);
extern void CheckLoadException(const char *json, const char *expectedMessage);

extern void CheckException(std::function<void()> lambda, const char *expectedMsg);