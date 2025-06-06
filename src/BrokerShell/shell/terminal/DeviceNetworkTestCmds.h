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

namespace dcclite::broker::exec::dcc
{
	class NetworkTask;
}

namespace dcclite::broker::shell::terminal
{	
	class StartNetworkTestCmd: public TerminalCmd
	{
		public:
			/// <summary>
			/// Start a network test task: Start-NetworkTest <device-path>
			/// </summary>
			/// 
			/// Returns the task id
			/// 
			/// <param name="name"></param>
			explicit StartNetworkTestCmd(RName name = RName{ "Start-NetworkTest" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	class StopNetworkTestCmd: public TerminalCmd
	{
		public:
			/// <summary>
			/// Stops a Network test task: Stop-NetworkTest <task-id>
			/// </summary>
			/// <param name="name"></param>
			explicit StopNetworkTestCmd(RName name = RName{ "Stop-NetworkTest" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};

	class ReceiveNetworkTestDataCmd: public TerminalCmd
	{
		public:
			/// <summary>
			/// Grabs the current data from an existing network test: Receive-NetworkTestData <task-id>
			/// </summary>
			/// <param name="name"></param>
			explicit ReceiveNetworkTestDataCmd(RName name = RName{ "Receive-NetworkTestData" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override;
	};
}
