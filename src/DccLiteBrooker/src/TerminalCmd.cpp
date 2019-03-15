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
	throw std::logic_error(fmt::format("Cannot add childs to TerminalCmdHost, sorry, obj name {}", obj->GetName()));
}

void TerminalCmdHost::AddCmd(std::unique_ptr<TerminalCmd> cmd)
{
	FolderObject::AddChild(std::move(cmd));
}

TerminalCmd *TerminalCmdHost::TryFindCmd(std::string_view name)
{
	return static_cast<TerminalCmd *>(this->TryGetChild(name));
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

