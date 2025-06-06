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

#include <map>
#include <optional>
#include <string_view>

#include <dcclite_shared/SharedLibDefs.h>

namespace dcclite
{
	class Guid;
	class RName;
}

namespace dcclite::broker::exec::dcc
{
	class Decoder;
	class Device;	

	namespace StorageManager
	{
		typedef std::map<RName, dcclite::DecoderStates> DecodersMap_t;

		void SaveState(const Device &device);
		DecodersMap_t LoadState(RName deviceName, const dcclite::Guid expectedToken);

		dcclite::Guid GetFileToken(const std::string_view fileName);
	}
}
