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

#include <wx/cmdproc.h>
#include <wx/rtti.h>

#include <stdexcept>

#include "EditCmds.h"
#include "MapObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(LitePanel::Gui::PanelDocument, wxDocument);

constexpr unsigned MAX_EDIT_CMDS = 3;
typedef std::array<std::unique_ptr<LitePanel::EditCmd>, MAX_EDIT_CMDS> CmdsArray_t;

class PanelDocumentCommand : public wxCommand
{
	public:	
		PanelDocumentCommand(
			LitePanel::Gui::PanelDocument &doc, 
			const wxString &name, 
			std::unique_ptr<LitePanel::EditCmd> cmd1, 
			std::unique_ptr<LitePanel::EditCmd> cmd2 = {}, 
			std::unique_ptr<LitePanel::EditCmd> cmd3 = {}
		);

	protected:
		bool Do() override;
		bool Undo() override;

	private:
		CmdsArray_t m_arCmds;
		CmdsArray_t m_arUndoCmds;

		LitePanel::Gui::PanelDocument &m_rclDocument;
};

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
	return this->Do();	
}

namespace LitePanel::Gui
{	
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
		if (!wxDocument::OnCreate(path, flags))
			return false;

		m_upPanel = std::make_unique<LitePanel::Panel>(LitePanel::TileCoord_t{ 32, 32 });

		return true;
	}

}