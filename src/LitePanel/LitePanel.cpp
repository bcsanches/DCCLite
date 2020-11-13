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

#include <wx/cmdproc.h>
#include <wx/docview.h>
#include <wx/object.h>

#include <fmt/format.h>

#include "EditCmds.h"
#include "LitePanel.h"
#include "MapObject.h"
#include "Panel.h"
#include "PanelDocument.h"
#include "PanelDocumentView.h"
#include "PanelEditorCanvas.h"
#include "RailObject.h"
#include "TempObjects.h"

wxIMPLEMENT_APP(LitePanel::Gui::LiteApp);

namespace LitePanel::Gui
{

	typedef  std::function<void(const LitePanel::TileCoord_t &)> ToolProc_t;

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
			wxToolBar &m_wxToolBar;


			ToolProc_t		m_pfnSelectedTool = nullptr;

			struct ToolInfo
			{
				wxToolBarToolBase *m_pTool;
				ToolProc_t m_pfnProc;
			};

			std::vector<ToolInfo> m_vecTools;
	};

	ToolManager::ToolManager(wxToolBar &toolBar) :
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
			if (tool.m_pTool == toolInfo.m_pTool)
				continue;

			m_wxToolBar.ToggleTool(tool.m_pTool->GetId(), false);
		}
	}

	class MainFrame : public wxDocParentFrame
	{
		public:
			MainFrame(wxDocManager *manager);

			void SetCurrentView(PanelDocumentView &view);

		private:
			void OnExit(wxCommandEvent &event);
			void OnAbout(wxCommandEvent &event);

			void OnToolLeftClick(wxCommandEvent &event);			

			void OnTileLeftClick(LitePanel::Gui::TileEvent &event);
			void OnTileUnderMouseChanged(LitePanel::Gui::TileEvent &event);

			void ExecuteInsertRailCmd(std::unique_ptr<LitePanel::RailObject> railObj);

		private:		
			std::unique_ptr<ToolManager> m_upToolManager;

			LitePanel::QuadObject *m_pclMouseShadow = nullptr;
			wxDocManager *m_pclDocManager = nullptr;

			wxMenu *m_pclEditMenu = nullptr;

			wxCommandProcessor *m_pclCommandProcessor = nullptr;

			PanelDocument *m_pclDocument = nullptr;

			LitePanel::Gui::PanelEditorCanvas *m_pclMapCanvas = nullptr;
	};
	
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

		m_pclMainFrame = new MainFrame(docManager);

		docManager->CreateNewDocument();

		m_pclMainFrame->Show(true);		

		return true;
	}

	void LiteApp::SetCurrentView(PanelDocumentView &view)
	{
		m_pclMainFrame->SetCurrentView(view);		
	}

	void LiteApp::CreateChildFrame(PanelDocumentView &view)
	{
		auto subFrame = new wxDocChildFrame(
			view.GetDocument(),
			&view,
			wxStaticCast(GetTopWindow(), wxDocParentFrame),
			wxID_ANY,
			"child frame",
			wxDefaultPosition,
			wxSize(300, 300)
		);

		subFrame->Centre();
	}

	void MainFrame::SetCurrentView(PanelDocumentView &view)
	{
		m_pclDocument = static_cast<PanelDocument *>(view.GetDocument());

		m_pclCommandProcessor = m_pclDocument->GetCommandProcessor();
		m_pclCommandProcessor->SetEditMenu(m_pclEditMenu);
		m_pclCommandProcessor->Initialize();			

		m_pclMapCanvas->SetTileMap(&m_pclDocument->GetPanel()->GetTileMap());
	}

	void MainFrame::ExecuteInsertRailCmd(std::unique_ptr<LitePanel::RailObject> railObj)
	{
		const auto &coord = railObj->GetPosition();		

		m_pclCommandProcessor->Submit(
			new PanelDocumentCommand{
				*m_pclDocument,
				"Insert rail object",
				m_pclDocument->GetPanel()->IsRailTileOccupied(coord) ? std::make_unique<LitePanel::RemoveRailCmd>(coord) : std::unique_ptr<LitePanel::EditCmd>{},
				std::make_unique<LitePanel::InsertRailCmd>(std::move(railObj))
			}
		);		
	}

	MainFrame::MainFrame(wxDocManager *docManager) :
		wxDocParentFrame(docManager, nullptr, wxID_ANY, "Lite Panel"),		
		m_pclDocManager(docManager)
	{
		assert(docManager);

		wxMenu *menuFile = new wxMenu;

		menuFile->Append(wxID_NEW);
		menuFile->Append(wxID_OPEN);
		menuFile->Append(wxID_CLOSE);
		menuFile->Append(wxID_SAVE);
		menuFile->Append(wxID_SAVEAS);

		menuFile->AppendSeparator();
		menuFile->Append(wxID_EXIT);

		m_pclEditMenu = new wxMenu;
		m_pclEditMenu->Append(wxID_UNDO);
		m_pclEditMenu->Append(wxID_REDO);

		wxMenu *menuHelp = new wxMenu;
		menuHelp->Append(wxID_ABOUT);

		wxMenuBar *menuBar = new wxMenuBar;
		menuBar->Append(menuFile, "&File");
		menuBar->Append(m_pclEditMenu, "&Edit");
		menuBar->Append(menuHelp, "&Help");
		
		m_pclMapCanvas = new LitePanel::Gui::PanelEditorCanvas(this);

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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
			[this](const LitePanel::TileCoord_t &coord) -> void
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
		m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_LEFT_CLICK, &MainFrame::OnTileLeftClick, this, wxID_ANY);
		m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_UNDER_MOUSE_CHANGED, &MainFrame::OnTileUnderMouseChanged, this, wxID_ANY);
	}


	void MainFrame::OnExit(wxCommandEvent &event)
	{
		Close(true);
	}

	void MainFrame::OnAbout(wxCommandEvent &event)
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


	void MainFrame::OnTileLeftClick(LitePanel::Gui::TileEvent &event)
	{
		auto tilePos = event.GetTilePosition();

		if (!tilePos)
		{
			//should never happen
			this->SetStatusText("No tile");

			return;
		}

		auto proc = m_upToolManager->TryGetSelectedToolProc();
		if (!proc)
			return;

		proc(tilePos.value());
	}

	void MainFrame::OnTileUnderMouseChanged(LitePanel::Gui::TileEvent &event)
	{
		event.Skip();

		const auto &position = event.GetTilePosition();

		auto panel = m_pclDocument->GetPanel();

		if (!position)
		{
			if (m_pclMouseShadow)
			{
				panel->UnregisterTempObject(*m_pclMouseShadow);
				m_pclMouseShadow = nullptr;
			}
		}
		else
		{
			if (!m_pclMouseShadow)
			{
				auto obj = std::make_unique<LitePanel::QuadObject>(position.value(), 1.0f, 1.0f, 0.7f);

				m_pclMouseShadow = obj.get();

				panel->RegisterTempObject(std::move(obj));
			}
			else
			{
				panel->SetTempObjectPosition(*m_pclMouseShadow, position.value());
			}
		}
	}

}