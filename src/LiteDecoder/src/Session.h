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

#include <stdint.h>

class EpromStream;

namespace dcclite
{
	class Guid;
}

namespace Session
{
	extern void LoadConfig(EpromStream &stream);
	extern void SaveConfig(EpromStream &stream);

	extern bool Init();

	extern bool Configure(const uint8_t *srvIp, uint16_t srvport);

	extern void Update();

	extern void LogStatus();

	extern const dcclite::Guid &GetConfigToken();

	//This is to be only called by DecoderManager when loading its data
	extern void ReplaceConfigToken(const dcclite::Guid &configToken);
}
