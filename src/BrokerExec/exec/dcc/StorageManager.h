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

		/**
		* This reads the hash stored on cache to check if the current configuration file changed 
		*
		* If it did, it generates a new token and stores the new hash on cache, if not, it returns the stored token.
		* 
		* @returns the token associated with the file, or a new one if the file changed since last check, or null if there was an error reading the file or cache
		* 				
		*/
		dcclite::Guid GetFileToken(const std::string_view fileName);
	}
}
