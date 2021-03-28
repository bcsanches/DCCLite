// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TerminalCmd.h"

#include <fmt/format.h>

#include <map>
#include <sstream>

#include "SpecialFolders.h"

void TerminalContext::SetLocation(const dcclite::Path_t &newLocation)
{
#if 1
	if (newLocation.is_relative())
	{		
		throw std::invalid_argument(fmt::format("TerminalContext::SetLocation->cannot use relative path: {}", newLocation.string()));
	}
#endif

	m_pthLocation = newLocation;
}

dcclite::IObject *TerminalContext::GetItem() const
{
	return m_rclRoot.TryNavigate(m_pthLocation);
}

TerminalCmdHost::TerminalCmdHost():
	FolderObject(SpecialFolders::GetName(SpecialFolders::Folders::CmdHostId))
{
	//empty
}

TerminalCmdHost::~TerminalCmdHost()
{
	//empty
}

dcclite::IObject *TerminalCmdHost::AddChild(std::unique_ptr<IObject> obj)
{
	throw std::logic_error(fmt::format("[TerminalCmdHost::AddChild] Cannot add childs, sorry, obj name {}", obj->GetName()));
}

TerminalCmd *TerminalCmdHost::AddCmd(std::unique_ptr<TerminalCmd> cmd)
{
	auto tmp = cmd.get();

	FolderObject::AddChild(std::move(cmd));

	return tmp;
}

void TerminalCmdHost::AddAlias(std::string name, TerminalCmd &target)
{
	if (target.GetParent() != this)
	{
		throw std::logic_error(fmt::format("[TerminalCmdHost::AddAlias] Invalid parent for {}", target.GetName()));
	}

	FolderObject::AddChild(std::make_unique<dcclite::Shortcut>(std::move(name), target));
}

TerminalCmd *TerminalCmdHost::TryFindCmd(std::string_view name)
{
	return static_cast<TerminalCmd *>(this->TryResolveChild(name));
}

TerminalCmd::TerminalCmd(std::string name):
	IObject(std::move(name))
{
	//empty
}

TerminalCmd::~TerminalCmd()
{
	//empty
}

