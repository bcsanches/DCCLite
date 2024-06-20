// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <array>

#include <wx/cmdproc.h>
#include <wx/docview.h>

#include <Panel.h>

#include "LitePanelLibDefs.h"
#include "EditCmds.h"

namespace LitePanel::Gui
{
	constexpr unsigned MAX_EDIT_CMDS = 3;
	typedef std::array<std::unique_ptr<LitePanel::EditCmd>, MAX_EDIT_CMDS> CmdsArray_t;

	class PanelDocument;

	class PanelDocumentCommand : public wxCommand
	{
		public:
			PanelDocumentCommand(
				PanelDocument &doc,
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

	class PanelDocument: public wxDocument
	{
		public:
			PanelDocument();
			virtual ~PanelDocument();

			bool OnCreate(const wxString &path, long flags) override;

			wxDECLARE_DYNAMIC_CLASS(PanelDocument);

			inline LitePanel::Panel *GetPanel()
			{
				return m_upPanel.get();
			}

		protected:
			bool DoSaveDocument(const wxString& filename) override;
			bool DoOpenDocument(const wxString& filename) override;

		private:
			std::unique_ptr<LitePanel::Panel> m_upPanel;
	};
}


