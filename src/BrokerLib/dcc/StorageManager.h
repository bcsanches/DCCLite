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
#include <string_view>

#include "SharedLibDefs.h"

namespace dcclite
{
	class Guid;
	class RName;
}

namespace dcclite::broker
{
	class Decoder;
	class Device;
	class Project;	

	namespace StorageManager
	{
		void SaveState(const Device &device, const Project &project);
		void LoadState(RName deviceName, const Project &project, const dcclite::Guid expectedToken);

		std::optional<DecoderStates> TryGetStoredState(Decoder &decoder);

		dcclite::Guid GetFileToken(const std::string_view fileName, const Project &project);
	}
}
