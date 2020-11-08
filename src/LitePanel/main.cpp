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

#include <wx/docview.h>
#include <wx/object.h>

#include <fmt/format.h>

#include "EditCmds.h"
#include "MapObject.h"
#include "Panel.h"
#include "PanelDocument.h"
#include "PanelDocumentView.h"
#include "PanelEditorCanvas.h"
#include "RailObject.h"
#include "TempObjects.h"

typedef  std::function<void(const LitePanel::TileCoord_t&)> ToolProc_t;

class ToolManager
{
	public:
		ToolManager(wxToolBar &toolBar);

		void AddTool(					
					const wxString &label,
					const wxBitmap &bitmap,		
					ToolProc_t proc,
					const wxString &shortHelp = wxEmptyString,
					const wxString &longHelp = wxEmptyString
		);

		void OnLeftClick(wxCommandEvent &event);

		inline ToolProc_t TryGetSelectedToolProc() const
		{
			return m_pfnSelectedTool;
		}

	private:
		wxToolBar		&m_wxToolBar;


		ToolProc_t		m_pfnSelectedTool = nullptr;

		struct ToolInfo
		{
			wxToolBarToolBase *m_pTool;
			ToolProc_t m_pfnProc;
		};

		std::vector<ToolInfo> m_vecTools;		
};

ToolManager::ToolManager(wxToolBar &toolBar):
	m_wxToolBar(toolBar)
{
	//empty
}

void ToolManager::AddTool(	
	const wxString &label,
	const wxBitmap &bitmap,
	ToolProc_t proc,
	const wxString &shortHelp,
	const wxString &longHelp)
{	
	auto tool = m_wxToolBar.AddCheckTool(wxID_ANY, label, bitmap, wxNullBitmap, shortHelp, longHelp);			

	m_vecTools.push_back(ToolInfo{ tool, proc });
}

void ToolManager::OnLeftClick(wxCommandEvent &event)
{
	auto clickedToolIt = std::find_if(
		m_vecTools.begin(), 
		m_vecTools.end(),
		[id = event.GetId()](auto &toolInfo)
		{
			return toolInfo.m_pTool->GetId() == id;
		}
	);

	//assert(clickedToolIt != m_vecTools.end());
	if (clickedToolIt == m_vecTools.end())
	{
		event.Skip();
		return;
	}

	auto &toolInfo = *clickedToolIt;

	if (!toolInfo.m_pTool->IsToggled())
	{
		m_pfnSelectedTool = nullptr;

		return;
	}		
	else
	{
		m_pfnSelectedTool = toolInfo.m_pfnProc;
	}

	for (auto &tool : m_vecTools)
	{
		if(tool.m_pTool == toolInfo.m_pTool)
			continue;
		
		m_wxToolBar.ToggleTool(tool.m_pTool->GetId(), false);		
	}
}

class LiteApp : public wxApp
{
	public:
		virtual bool OnInit();
};

class MainFrame : public wxDocParentFrame
{
	public:
		MainFrame(wxDocManager *manager);

	private:
		void OnHello(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);	

		void OnToolLeftClick(wxCommandEvent &event);
		void OnMapCanvasLeftClick(wxMouseEvent &event);

		void OnMapCanvasTileLeftClick(LitePanel::Gui::TileEvent &event);

		void OnTileUnderMouseChanged(LitePanel::Gui::TileEvent& event);

		void ExecuteInsertRailCmd(std::unique_ptr<LitePanel::RailObject> railObj);

	private:
		LitePanel::Panel m_clPanel;

		std::unique_ptr<ToolManager> m_upToolManager;

		LitePanel::Gui::TileMapCanvas *m_pclMapCanvas = nullptr;

		LitePanel::QuadObject *m_pclMouseShadow = nullptr;

		LitePanel::EditCmdManager m_clEditCmdManager;
};



wxIMPLEMENT_APP(LiteApp);
bool LiteApp::OnInit()
{
	auto docManager = new wxDocManager();
	docManager->SetMaxDocsOpen(1);

	new wxDocTemplate(
		docManager,
		"Panel files",		//short description
		"*.panel",			//filter
		"panel",			//default folder
		"panel",			//extension
		"Panel Json File",	//file identifier
		"Panel View",		//view identifier
		CLASSINFO(LitePanel::Gui::PanelDocument),
		CLASSINFO(LitePanel::Gui::PanelDocumentView)
	);

	auto *frame = new MainFrame(docManager);
	frame->Show(true);

	return true;
}


void MainFrame::ExecuteInsertRailCmd(std::unique_ptr<LitePanel::RailObject> railObj)
{
	const auto &coord = railObj->GetPosition();

	LitePanel::ComplexEditCmd cmd{
				"Insert rail object",
				m_clPanel.IsRailTileOccupied(coord) ? std::make_unique<LitePanel::RemoveRailCmd>(coord) : std::unique_ptr<LitePanel::EditCmd>{},
				std::make_unique<LitePanel::InsertRailCmd>(std::move(railObj))
	};

	m_clEditCmdManager.Run(std::move(cmd), m_clPanel);
}

MainFrame::MainFrame(wxDocManager *docManager):
	wxDocParentFrame(docManager, nullptr, wxID_ANY, "Lite Panel"),
	m_clPanel(LitePanel::TileCoord_t{32, 32})
{
	wxMenu* menuFile = new wxMenu;
	
	menuFile->Append(wxID_NEW);
	menuFile->Append(wxID_OPEN);
	menuFile->Append(wxID_CLOSE);
	menuFile->Append(wxID_SAVE);
	menuFile->Append(wxID_SAVEAS);

	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu *const menuEdit = new wxMenu;
	menuEdit->Append(wxID_UNDO);
	menuEdit->Append(wxID_REDO);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuEdit, "&Edit");
	menuBar->Append(menuHelp, "&Help");	

	m_pclMapCanvas = new LitePanel::Gui::PanelEditorCanvas(this);

	m_pclMapCanvas->SetTileMap(&m_clPanel.GetTileMap());

	auto obj = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{1, 1},
		LitePanel::ObjectAngles::EAST,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clPanel.RegisterRail(std::move(obj));

	auto obj2 = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{ 2, 1 },
		LitePanel::ObjectAngles::NORTHEAST,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clPanel.RegisterRail(std::move(obj2));

	auto obj3 = std::make_unique<LitePanel::SimpleRailObject>(
		LitePanel::TileCoord_t{ 3, 1 },
		LitePanel::ObjectAngles::NORTH,
		LitePanel::SimpleRailTypes::STRAIGHT
	);

	m_clPanel.RegisterRail(std::move(obj3));

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to LitePanel Editor!");

	auto toolBar = this->CreateToolBar(wxTB_TOP | wxTB_FLAT | wxTB_DOCKABLE);

	m_upToolManager = std::make_unique<ToolManager>(*toolBar);	

	m_upToolManager->AddTool(		
		"track", 
		wxBITMAP(rail_straight_000_icon),
		[this](const LitePanel::TileCoord_t &coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			));			
		},
		"Horizontal track section",
		"Creates a horizontal track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_045_icon),
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::NORTHEAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			));			
		},
		"Diagonal track section",
		"Creates a diagonal track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_090_icon),	
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::STRAIGHT
			));
		},
		"Vertical track section",
		"Creates a vertical track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_straight_135_icon),
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::NORTHWEST,
				LitePanel::SimpleRailTypes::STRAIGHT
				));
		},
		"Diagonal inverted track section",
		"Creates a inverted diagonal track section"
	);

	toolBar->AddSeparator();

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_000_icon),		
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			));
		},
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_090_icon),	
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			));
		},
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_180_icon),		
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			));
		},
		"Left curve track section",
		"Creates a left curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_left_curve_270_icon),		
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::SOUTH,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			));
		},
		"Left curve track section",
		"Creates a left curve track section"
	);

	toolBar->AddSeparator();

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_000_icon),	
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			));
		},
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_090_icon),		
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			));
		},
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_180_icon),	
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			));
		},
		"Right curve track section",
		"Creates a right curve track section"
	);

	m_upToolManager->AddTool(		
		"track",
		wxBITMAP(rail_right_curve_270_icon),
		[this](const LitePanel::TileCoord_t& coord) -> void
		{
			ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
				coord,
				LitePanel::ObjectAngles::SOUTH,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			));
		},
		"Right curve track section",
		"Creates a right curve track section"
	);

	toolBar->AddSeparator();

	toolBar->Realize();
	//toolBar->SetRows(1);
	
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnToolLeftClick, this, wxID_ANY);
	m_pclMapCanvas->Bind(wxEVT_LEFT_UP, &MainFrame::OnMapCanvasLeftClick, this, wxID_ANY);
	m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_LEFT_CLICK, &MainFrame::OnMapCanvasTileLeftClick, this, wxID_ANY);
	m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_UNDER_MOUSE_CHANGED, &MainFrame::OnTileUnderMouseChanged, this, wxID_ANY);
}


void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(
		"This LitePanel editor\n"
		"\n"
		"Author: Bruno C. Sanches - 2020",
		"About LitePanel", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnToolLeftClick(wxCommandEvent &event)
{
	//wxLogMessage("Hello toolbar!");

	m_upToolManager->OnLeftClick(event);
}


void MainFrame::OnMapCanvasLeftClick(wxMouseEvent &event)
{
	auto tilePos = m_pclMapCanvas->TryFindMouseTile(event);

	if (!tilePos)
	{
		this->SetStatusText("No tile");

		return;
	}
	
	auto proc = m_upToolManager->TryGetSelectedToolProc();
	if (!proc)
		return;

	proc(tilePos.value());
}

void MainFrame::OnMapCanvasTileLeftClick(LitePanel::Gui::TileEvent &event)
{
	const auto &tilePos = event.GetTilePosition().value();

	this->SetStatusText(fmt::format("tile {} {}", tilePos.m_tX, tilePos.m_tY));
}

void MainFrame::OnTileUnderMouseChanged(LitePanel::Gui::TileEvent &event)
{
	event.Skip();

	const auto &position = event.GetTilePosition();

	if (!position)
	{
		if (m_pclMouseShadow)
		{
			m_clPanel.UnregisterTempObject(*m_pclMouseShadow);
			m_pclMouseShadow = nullptr;
		}
	}
	else
	{
		if (!m_pclMouseShadow)
		{
			auto obj = std::make_unique<LitePanel::QuadObject>(position.value(), 1.0f, 1.0f, 0.7f);

			m_pclMouseShadow = obj.get();

			m_clPanel.RegisterTempObject(std::move(obj));
		}
		else
		{
			m_clPanel.SetTempObjectPosition(*m_pclMouseShadow, position.value());
		}
	}
}
