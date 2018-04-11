#include "TerminalCmd.h"

#include <map>
#include <sstream>

static std::map<std::string_view, TerminalCmd *> GetCmdMap()
{
	static std::map<std::string_view, TerminalCmd *> g_mapCmds;

	return g_mapCmds;
}

TerminalCmd::TerminalCmd(std::string_view name):
	m_strName(name)
{
	auto &mapCmds = GetCmdMap();

	auto it = mapCmds.find(name);

	if (it != mapCmds.end())
	{
		std::stringstream stream;

		stream << "TerminalCmd " << name << " already exists";

		throw std::runtime_error(stream.str());
	}

	mapCmds.insert(it, std::make_pair(m_strName, this));
}

TerminalCmd::~TerminalCmd()
{
	auto &mapCmds = GetCmdMap();

	auto it = mapCmds.find(m_strName);

	if (it != mapCmds.end())
		mapCmds.erase(it);
}


