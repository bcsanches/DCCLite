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
#include <memory>
#include <string>

#include "Object.h"
#include "Project.h"
#include "Service.h"

namespace dcclite
{
	class Clock;
}

class TerminalCmdHost;

class Broker
{
	private:	
		dcclite::FolderObject m_clRoot;
		dcclite::FolderObject *m_pServices;

		Project m_clProject;

	private:

		void LoadConfig();

	public:
		Broker(std::filesystem::path projectPath);

		void Update(const dcclite::Clock &clock);
};
