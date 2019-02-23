#include "TerminalCmd.h"

#include <fmt/format.h>

#include <map>
#include <sstream>

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

static std::map<std::string_view, TerminalCmd *> &GetCmdMap()
{
	static std::map<std::string_view, TerminalCmd *> g_mapCmds;

	return g_mapCmds;
}

TerminalCmd::TerminalCmd(std::string_view name):
	m_strName(name)
{
	auto &mapCmds = GetCmdMap();

	auto it = mapCmds.find(m_strName);

	if (it != mapCmds.end())
	{
		std::stringstream stream;

		stream << "TerminalCmd " << name << " already exists";

		throw std::runtime_error(stream.str());
	}

	mapCmds.insert(it, std::make_pair(std::string_view(m_strName), this));
}

TerminalCmd::~TerminalCmd()
{
	auto &mapCmds = GetCmdMap();

	auto it = mapCmds.find(m_strName);

	if (it != mapCmds.end())
		mapCmds.erase(it);
}

TerminalCmd *TerminalCmd::TryFindCmd(std::string_view name)
{
	auto &mapCmds = GetCmdMap();

	auto it = mapCmds.find(name);

	return it != mapCmds.end() ? it->second : nullptr;
}


