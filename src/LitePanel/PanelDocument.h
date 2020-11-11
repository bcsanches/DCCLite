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

#include <wx/docview.h>

#include <Panel.h>

namespace LitePanel::Gui
{
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

		private:
			std::unique_ptr<LitePanel::Panel> m_upPanel;
	};
}


