#pragma once

#include <string_view>

class TerminalCmd
{
	public:
		TerminalCmd(std::string_view name);

		TerminalCmd(const TerminalCmd &) = delete;
		TerminalCmd(TerminalCmd &&) = delete;

		~TerminalCmd();

	private:
		std::string m_strName;
};
