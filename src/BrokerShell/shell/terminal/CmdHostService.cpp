// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CmdHostService.h"

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>

#include "sys/ServiceFactory.h"

#include "TerminalCmd.h"
#include "TerminalCmdProvider.h"

namespace dcclite::broker::shell::terminal
{
	CmdHostService::CmdHostService(RName name, sys::Broker &broker, const rapidjson::Value &params):
		Service(name, broker, params)
	{
		//empty
	}

	dcclite::IObject *CmdHostService::AddChild(std::unique_ptr<Object> obj)
	{
		throw std::logic_error(fmt::format("[CmdHost::AddChild] Cannot add childs, sorry, obj name {}", obj->GetName()));
	}

	TerminalCmd *CmdHostService::AddCmd(std::unique_ptr<TerminalCmd> cmd)
	{
		auto tmp = cmd.get();

		FolderObject::AddChild(std::move(cmd));

		return tmp;
	}

	void CmdHostService::AddAlias(RName name, TerminalCmd &target)
	{
		if (target.GetParent() != this)
		{
			throw std::logic_error(fmt::format("[TerminalCmdHost::AddAlias] Invalid parent for {}", target.GetName()));
		}

		FolderObject::AddChild(std::make_unique<dcclite::Shortcut>(name, target));
	}

	TerminalCmd *CmdHostService::TryFindCmd(RName name)
	{
		return static_cast<TerminalCmd *>(this->TryResolveChild(name));
	}


	void CmdHostService::OnLoadFinished()
	{
		dcclite::Log::Trace("[CmdHostService::OnLoadFinished] Registering terminal commands from providers");

		m_rclBroker.VisitServices([this](IObject &svc)
			{
				auto *cmdProvider = dynamic_cast<ITerminalCmdProvider *>(&svc);

				if (cmdProvider != nullptr)
				{
					dcclite::Log::Trace("[CmdHostService::OnLoadFinished] Registering terminal commands for {}", svc.GetName());

					cmdProvider->ITerminalCmdProvider_RegisterLocalCmds(*this);
				}

				return true;
			}
		);

		dcclite::Log::Trace("[CmdHostService::OnLoadFinished] done - commands registered");
	}

	void CmdHostService::OnUnload()
	{
		//nothing todo
	}

	void CmdHostService::RegisterFactory()
	{
		static sys::GenericServiceFactory<CmdHostService> g_clFactory;
	}

	const char *CmdHostService::TYPE_NAME = "CmdHostService";
}
