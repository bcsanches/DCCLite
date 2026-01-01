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

#include <rapidjson/document.h>

#include <dcclite/FolderObject.h>

namespace dcclite::broker::tycoon
{	
	class TycoonService;

	class Location : public dcclite::FolderObject
	{
		public:
			Location(RName name, TycoonService &tycoon, const rapidjson::Value &params);

			IObject *AddChild(std::unique_ptr<Object> obj) override;

		private:			
	};
}
