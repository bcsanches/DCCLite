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
#include "RailObject.h"

class ToolManager
{
	public:
		ToolManager(wxToolBar &toolBar);

		void AddTool(					
					const wxString &label,
					const wxBitmap &bitmap,			
					const wxString &shortHelp = wxEmptyString,
					const wxString &longHelp = wxEmptyString
		);

		void OnLeftClick(wxCommandEvent &event);

	private:
		wxToolBar		&m_wxToolBar;

		std::vector<wxToolBarToolBase *> m_vecTools;
};

ToolManager::ToolManager(wxToolBar &toolBar):
	m_wxToolBar(toolBar)
{
	//empty
}

void ToolManager::AddTool(	
	const wxString &label,
	const wxBitmap &bitmap,
	const wxString &shortHelp,
	const wxString &longHelp)
{	
	auto tool = m_wxToolBar.AddCheckTool(wxID_ANY, label, bitmap, wxNullBitmap, shortHelp, longHelp);			

	m_vecTools.push_back(tool);	
}

void ToolManager::OnLeftClick(wxCommandEvent &event)
{
	auto clickedTool = std::find_if(
		m_vecTools.begin(), 
		m_vecTools.end(),
		[id = event.GetId()](auto *tool)
		{
			return tool->GetId() == id;
		}
	);

	assert(clickedTool != m_vecTools.end());

	if(!(*clickedTool)->IsToggled())
		return;

	for (auto tool : m_vecTools)
	{
		if(tool == *clickedTool)
			continue;
		
		m_wxToolBar.ToggleTool(tool->GetId(), false);		
	}
}

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

		void OnToolLeftClick(wxCommandEvent &event);

	private:
		LitePanel::TileMap m_clTileMap;

		std::unique_ptr<ToolManager> m_upToolManager;
};

enum
{
	ID_Hello = 1,
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

	auto obj = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{1, 1},
		LitePanel::ObjectAngles::EAST,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clTileMap.RegisterObject(std::move(obj), 0);

	auto obj2 = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{ 2, 1 },
		LitePanel::ObjectAngles::NORTHEAST,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clTileMap.RegisterObject(std::move(obj2), 0);

	auto obj3 = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{ 3, 1 },
		LitePanel::ObjectAngles::NORTH,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clTileMap.RegisterObject(std::move(obj3), 0);

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to LitePanel Editor!");

	auto toolBar = this->CreateToolBar(wxTB_TOP | wxTB_FLAT | wxTB_DOCKABLE);

	m_upToolManager = std::make_unique<ToolManager>(*toolBar);	

	m_upToolManager->AddTool(		
		"track", 
		wxBITMAP(rail_straight_000_icon),
		"Horizontal track section",
		"Creates a horizontal track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_045_icon),
		"Diagonal track section",
		"Creates a diagonal track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_090_icon),		
		"Vertical track section",
		"Creates a vertical track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_135_icon),		
		"Diagonal inverted track section",
		"Creates a inverted diagonal track section"
	);

	toolBar->AddSeparator();

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_000_icon),		
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_090_icon),		
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_180_icon),		
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_270_icon),		
		"Left curve track section",
		"Creates a left curve track section"
	);

	toolBar->AddSeparator();

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_000_icon),		
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_090_icon),		
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_180_icon),		
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_270_icon),		
		"Right curve track section",
		"Creates a right curve track section"
	);

	toolBar->AddSeparator();




	toolBar->Realize();
	//toolBar->SetRows(1);

	Bind(wxEVT_MENU, &MainFrame::OnHello, this, ID_Hello);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnToolLeftClick, this, wxID_ANY);
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

void MainFrame::OnToolLeftClick(wxCommandEvent &event)
{
	//wxLogMessage("Hello toolbar!");

	m_upToolManager->OnLeftClick(event);
}
