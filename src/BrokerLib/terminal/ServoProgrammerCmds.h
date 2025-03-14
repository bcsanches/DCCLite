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

#include "ServiceCmdBase.h"

namespace dcclite::broker
{
	class NetworkTask;

	class StartServoProgrammerCmd: public DccLiteCmdBase
	{
		public:
			explicit StartServoProgrammerCmd(RName name = RName{ "Start-ServoProgrammer" }):
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};	

	class StopServoProgrammerCmd: public DccLiteCmdBase
	{
		public:
			explicit StopServoProgrammerCmd(RName name = RName{ "Stop-ServoProgrammer" }):
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	class EditServoProgrammerCmd: public DccLiteCmdBase
	{		
		public:
			explicit EditServoProgrammerCmd(RName name = RName{ "Edit-ServoProgrammer" }):
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	

	class DeployServoProgrammerCmd: public DccLiteCmdBase
	{
		public:
			explicit DeployServoProgrammerCmd(RName name = RName{ "Deploy-ServoProgrammer" }):
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};
}
