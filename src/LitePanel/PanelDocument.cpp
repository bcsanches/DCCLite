// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "PanelDocument.h"

#include <fstream>
#include <stdexcept>

#include <wx/rtti.h>

#include "JsonCreator/StringWriter.h"
#include "JsonCreator/Object.h"

#include "EditCmds.h"
#include "MapObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(LitePanel::Gui::PanelDocument, wxDocument);

namespace LitePanel::Gui
{
	PanelDocumentCommand::PanelDocumentCommand(
		LitePanel::Gui::PanelDocument &doc,
		const wxString &name,
		std::unique_ptr<LitePanel::EditCmd> cmd1,
		std::unique_ptr<LitePanel::EditCmd> cmd2,
		std::unique_ptr<LitePanel::EditCmd> cmd3
	) :
		wxCommand(true, name),		
		m_rclDocument(doc),
		m_arCmds({ std::move(cmd1), std::move(cmd2), std::move(cmd3) })
	{
		if (!m_arCmds[0].get() && !m_arCmds[1].get() && !m_arCmds[2].get())
			throw std::invalid_argument("[ComplexEditCmd::ComplexEditCmd] cmds cannot be empty");
	}

	bool PanelDocumentCommand::Do() 
	{
		auto &panel = *m_rclDocument.GetPanel();	

		for (auto i = 0; i < MAX_EDIT_CMDS; ++i)
		{
			auto cmd = m_arCmds[i].get();
			if (!cmd)
			{
				//make sure slot is empty, because on undo this can get dirty
				m_arUndoCmds[i].release();

				continue;
			}

			m_arUndoCmds[i] = cmd->Run(panel);
		}

		std::reverse(m_arUndoCmds.begin(), m_arUndoCmds.end());

		return true;
	}

	bool PanelDocumentCommand::Undo()
	{
		std::swap(m_arCmds, m_arUndoCmds);
		this->Do();	
		std::swap(m_arCmds, m_arUndoCmds);

		return true;
	}

	//
	//
	// PanelDocument
	//
	//

	PanelDocument::PanelDocument()
	{
		//empty
	}

	PanelDocument::~PanelDocument()
	{
		//empty
	}

	bool PanelDocument::OnCreate(const wxString &path, long flags)
	{
		m_upPanel = std::make_unique<LitePanel::Panel>(LitePanel::TileCoord_t{ 32, 32 });

		return wxDocument::OnCreate(path, flags);
	}

	bool PanelDocument::DoSaveDocument(const wxString &filename)
	{
		std::ofstream newStateFile(filename.c_str().AsWChar(), std::ios_base::trunc);

		JsonCreator::StringWriter responseWriter;

		{
			auto rootObj = JsonCreator::MakeObject(responseWriter);
			
			m_upPanel->Save(rootObj);			
		}

		newStateFile << responseWriter.GetString();

		return true;
	}

	bool PanelDocument::DoOpenDocument(const wxString &filename)
	{
		return false;
	}

}