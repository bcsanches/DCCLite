// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "EditCmds.h"

#include <fmt/format.h>
#include <stdexcept>

namespace LitePanel
{
	ComplexEditCmd::ComplexEditCmd(std::string description, std::unique_ptr<EditCmd> cmd1, std::unique_ptr<EditCmd> cmd2, std::unique_ptr<EditCmd> cmd3):
		m_strDescription(std::move(m_strDescription)),
		m_arCmds({ std::move(cmd1), std::move(cmd2), std::move(cmd3) })
	{		
		if(!m_arCmds[0].get())		
			throw std::invalid_argument("[ComplexEditCmd::ComplexEditCmd] cmds cannot be empty");		
	}

	ComplexEditCmd ComplexEditCmd::Run(Panel &panel)
	{
		CmdsArray_t undoStack;

		for (auto i = 0; i < MAX_EDIT_CMDS; ++i)
		{
			auto cmd = m_arCmds[i].get();
			if (!cmd)
				break;

			try
			{
				undoStack[i] = cmd->Run(panel);
			}			
			catch (...)
			{
				//first microcmd failed, so nothing todo... just blowup
				if (i == 0)
					throw;

				//try to rollback
				do
				{
					//if this blows up... we are doomed
					undoStack[i--]->Run(panel);					
				} while (i > 0);

				throw;
			}
		}

		std::reverse(undoStack.begin(), undoStack.end());

		return ComplexEditCmd(
			fmt::format("Undo {}", m_strDescription), 
			std::move(undoStack[0]), 
			std::move(undoStack[1]), 
			std::move(undoStack[2])
		);
	}

	void EditCmdManager::Run(ComplexEditCmd cmd, Panel &panel)
	{
		auto undoCmd = cmd.Run(panel);

		m_vecRedo.clear();
		m_vecUndo.push_back(std::move(undoCmd));
	}
}

