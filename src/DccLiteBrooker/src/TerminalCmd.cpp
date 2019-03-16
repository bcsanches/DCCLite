#include "TerminalCmd.h"

#include <fmt/format.h>

#include <map>
#include <sstream>

static TerminalCmdHost *g_CmdHost = nullptr;

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
	FolderObject("CmdHost")
{
	g_CmdHost = this;
}

TerminalCmdHost::~TerminalCmdHost()
{
	g_CmdHost = nullptr;
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

TerminalCmdHost *TerminalCmdHost::Instance()
{
	return g_CmdHost;
}


TerminalCmd::TerminalCmd(std::string name):
	IObject(name)
{
	//empty
}

TerminalCmd::~TerminalCmd()
{
	//empty
}

