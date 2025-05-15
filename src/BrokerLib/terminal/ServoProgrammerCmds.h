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

#include "TerminalCmd.h"

namespace dcclite::broker
{
	class NetworkTask;

	class StartServoProgrammerCmd: public TerminalCmd
	{
		public:
			explicit StartServoProgrammerCmd(RName name = RName{ "Start-ServoProgrammer" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};	

	class StopServoProgrammerCmd: public TerminalCmd
	{
		public:
			explicit StopServoProgrammerCmd(RName name = RName{ "Stop-ServoProgrammer" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	class EditServoProgrammerCmd: public TerminalCmd
	{		
		public:
			explicit EditServoProgrammerCmd(RName name = RName{ "Edit-ServoProgrammer" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	

	class DeployServoProgrammerCmd: public TerminalCmd
	{
		public:
			explicit DeployServoProgrammerCmd(RName name = RName{ "Deploy-ServoProgrammer" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};
}
