// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <wx/wx.h>

#include "MapCanvas.h"
#include "MapObject.h"

class LiteApp : public wxApp
{
	public:
		virtual bool OnInit();
};

class MainFrame : public wxFrame
{
	public:
		MainFrame();

	private:
		void OnHello(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);	

	private:
		LitePanel::TileMap m_clTileMap;
};

enum
{
	ID_Hello = 1
};

wxIMPLEMENT_APP(LiteApp);
bool LiteApp::OnInit()
{
	auto *frame = new MainFrame();
	frame->Show(true);

	return true;
}

MainFrame::MainFrame(): 
	wxFrame(NULL, wxID_ANY, "Lite Panel"),
	m_clTileMap(LitePanel::TileCoord_t{32, 32})
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
		"Help string shown in status bar for this menu item");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	auto oglCanvas = new LitePanel::MapCanvas(this);

	oglCanvas->SetTileMap(&m_clTileMap);

	auto obj = std::make_unique<LitePanel::MapObject>(LitePanel::TileCoord_t{1, 1});
	m_clTileMap.RegisterObject(std::move(obj), 0);

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to LitePanel Editor!");

	auto toolBar = this->CreateToolBar(wxTB_LEFT | wxTB_FLAT | wxTB_DOCKABLE);
	toolBar->AddTool(
		-1, 
		"track", 
		wxBITMAP(straight_icon),
		wxNullBitmap, 
		wxITEM_NORMAL, 
		"Horizontal track section",
		"Creates a horizontal track section"
	);

	toolBar->AddTool(
		-1,
		"track",
		wxBITMAP(straight_vertical_icon),
		wxNullBitmap,
		wxITEM_NORMAL,
		"Vertical track section",
		"Creates a vertical track section"
	);

	toolBar->Realize();
	//toolBar->SetRows(1);

	Bind(wxEVT_MENU, &MainFrame::OnHello, this, ID_Hello);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}


void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets Hello World example",
		"About Hello World", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnHello(wxCommandEvent& event)
{
	wxLogMessage("Hello world from wxWidgets!");
}
