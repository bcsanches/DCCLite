// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "PanelDocumentView.h"

#include "LitePanel.h"
#include "Panel.h"
#include "PanelDocument.h"
#include "PanelEditorCanvas.h"

#include <wx/rtti.h>

wxIMPLEMENT_DYNAMIC_CLASS(LitePanel::Gui::PanelDocumentView, wxView);

namespace LitePanel::Gui
{
	PanelDocumentView::PanelDocumentView()
	{
		//empty
	}

	PanelDocumentView::~PanelDocumentView()
	{
		//empty
	}

	bool PanelDocumentView::OnCreate(wxDocument *doc, long flags)
	{
		if (!wxView::OnCreate(doc, flags))
			return false;										

		auto panelDocument = static_cast<PanelDocument *>(doc);		

		auto &app = wxGetApp();
		app.SetCurrentView(*this);

		return true;		
	}

	bool PanelDocumentView::OnClose(bool deleteWindow)
	{
		if (!wxView::OnClose(deleteWindow))
			return false;

		auto &app = wxGetApp();
		app.RemoveCurrentView();

		return true;
	}

	void PanelDocumentView::OnDraw(wxDC *dc)
	{

	}
}
