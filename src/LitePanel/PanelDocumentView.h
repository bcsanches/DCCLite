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

namespace LitePanel::Gui
{
	class PanelDocument;
	class TileMapCanvas;

	class PanelDocumentView: public wxView
	{
		public:
			PanelDocumentView();
			virtual ~PanelDocumentView();

			PanelDocumentView(const PanelDocumentView &) = delete;
			PanelDocumentView(PanelDocumentView &&) = delete;			

		protected:
			bool OnClose(bool deleteWindow) override;

			void OnDraw(wxDC *dc) override;

			bool OnCreate(wxDocument *doc, long flags) override;

			wxDECLARE_DYNAMIC_CLASS(PanelDocumentView);		
	};
}


