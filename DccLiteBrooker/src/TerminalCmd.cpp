#include "TerminalCmd.h"

#include <map>
#include <sstream>

static std::map<std::string_view, TerminalCmd *> g_mapCmds;

TerminalCmd::TerminalCmd(std::string_view name):
	m_strName(name)
{
	auto it = g_mapCmds.find(name);

	if (it != g_mapCmds.end())
	{
		std::stringstream stream;

		stream << "TerminalCmd " << name << " already exists";

		throw std::runtime_error(stream.str());
	}

	g_mapCmds.insert(it, std::make_pair(m_strName, this));
}

TerminalCmd::~TerminalCmd()
{
	auto it = g_mapCmds.find(m_strName);

	if (it != g_mapCmds.end())
		g_mapCmds.erase(it);
}


