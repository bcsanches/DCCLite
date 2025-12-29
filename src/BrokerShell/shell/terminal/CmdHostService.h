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

#include "sys/Service.h"

namespace dcclite::broker::shell::terminal
{
	class TerminalCmd;

	/**
		The cmd host is responsbible for storing the registered cmds

		This was created to allow a common cmd repository that any subsystem can access and register its own commands

	*/
	class CmdHostService: public sys::Service, public sys::IPostLoadService
	{
		public:
			static void RegisterFactory();			

			static const char *TYPE_NAME;

			CmdHostService(RName name, sys::Broker &broker, const rapidjson::Value &params);

			IObject *AddChild(std::unique_ptr<Object> obj) override;

			TerminalCmd *AddCmd(std::unique_ptr<TerminalCmd> cmd);
			void AddAlias(RName name, TerminalCmd &target);

			TerminalCmd *TryFindCmd(RName name);

			const char *GetTypeName() const noexcept override
			{
				return "CmdHostService";
			}

			void OnLoadFinished() override;

			void OnUnload() override;
	};
}
