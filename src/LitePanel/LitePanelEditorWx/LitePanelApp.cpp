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
#include <wx/tglbtn.h>

#include <fmt/format.h>

#include <LitePanelLib/EditCmds.h>
#include <LitePanelLib/MapObject.h>
#include <LitePanelLib/Panel.h>
#include <LitePanelLib/RailObject.h>

#include <LitePanelLib/Render/ColorStyle.h>

#include "LitePanelApp.h"
#include "PanelDocument.h"
#include "PanelDocumentView.h"
#include "PanelEditorCanvas.h"

#include "TempObjects.h"

wxIMPLEMENT_APP(LitePanel::Gui::LiteApp);

namespace LitePanel::Gui
{
	typedef  std::function<void(const LitePanel::TileCoord_t &)> ToolProc_t;

	class ToolManager
	{
		public:			
			void AddTool(
				wxWindow *parent,
				wxSizer *sizer,
				const wxString &label,
				const wxBitmap &bitmap,
				ToolProc_t proc,
				const wxString &shortHelp = wxEmptyString,
				const wxString &longHelp = wxEmptyString
			);

			void OnToolToggle(wxCommandEvent &event);

			inline ToolProc_t TryGetSelectedToolProc() const
			{
				return m_pfnSelectedTool;
			}

		private:			
			struct ToolInfo
			{
				wxBitmapToggleButton *m_pButton;
				ToolProc_t m_pfnProc;				
			};

			ToolProc_t				m_pfnSelectedTool = nullptr;
			std::vector<ToolInfo>	m_vecTools;
	};

	void ToolManager::AddTool(
		wxWindow *parent,
		wxSizer *sizer,		
		const wxString &label,
		const wxBitmap &bitmap,
		ToolProc_t proc,
		const wxString &shortHelp,
		const wxString &longHelp)
	{
		auto button = new wxBitmapToggleButton(
			parent,
			wxID_ANY,
			bitmap,
			wxDefaultPosition,
			wxSize(32, 32)
		);

		if (!shortHelp.empty())
			button->SetToolTip(shortHelp);

		if (!longHelp.empty())
			button->SetHelpText(longHelp);

		sizer->Add(button, 0, wxALL, 0);

		m_vecTools.push_back({ button, std::move(proc) });

		button->Bind(wxEVT_TOGGLEBUTTON, &ToolManager::OnToolToggle, this);
	}

	void ToolManager::OnToolToggle(wxCommandEvent &event)
	{
		auto it = std::find_if(
			m_vecTools.begin(),
			m_vecTools.end(),
			[&event](const ToolInfo &info)
			{
				return info.m_pButton->GetId() == event.GetId();
			}
		);

		if (it == m_vecTools.end())
		{
			event.Skip();
			return;
		}

		if (!it->m_pButton->GetValue())
		{
			m_pfnSelectedTool = nullptr;
			return;
		}

		m_pfnSelectedTool = it->m_pfnProc;

		for (auto &tool : m_vecTools)
		{
			if (tool.m_pButton != it->m_pButton)
				tool.m_pButton->SetValue(false);
		}
	}

	class MainFrame : public wxDocParentFrame
	{
		public:
			MainFrame(wxDocManager *manager);

			void SetCurrentView(PanelDocumentView &view);
			void RemoveCurrentView();

		private:
			void OnExit(wxCommandEvent &event);
			void OnAbout(wxCommandEvent &event);			

			void OnTileLeftClick(LitePanel::Gui::TileEvent &event);
			void OnTileUnderMouseChanged(LitePanel::Gui::TileEvent &event);

			void ExecuteInsertRailCmd(std::unique_ptr<LitePanel::RailObject> railObj);
			void ExecuteInsertJunctionRailCmd(std::unique_ptr<LitePanel::JunctionRailObject> railObj);

			void InitTools(wxWindow *toolPalette, wxSizer *toolGrid);
			void InitMenuBar();

		private:		
			ToolManager m_clToolManager;

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
		view.SetFrame(m_pclMainFrame);
	}

	void LiteApp::RemoveCurrentView()
	{
		m_pclMainFrame->RemoveCurrentView();
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

	int LiteApp::OnExit()
	{
		delete wxDocManager::GetDocumentManager();

		return wxApp::OnExit();
	}

	void MainFrame::SetCurrentView(PanelDocumentView &view)
	{
		m_pclDocument = static_cast<PanelDocument *>(view.GetDocument());

		m_pclCommandProcessor = m_pclDocument->GetCommandProcessor();
		m_pclCommandProcessor->SetEditMenu(m_pclEditMenu);
		m_pclCommandProcessor->Initialize();			

		m_pclMapCanvas->SetTileMap(&m_pclDocument->GetPanel()->GetTileMap());
	}

	void MainFrame::RemoveCurrentView()
	{
		m_pclMapCanvas->SetTileMap(nullptr);

		m_pclCommandProcessor = nullptr;
		m_pclDocument = nullptr;
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

	void MainFrame::ExecuteInsertJunctionRailCmd(std::unique_ptr<LitePanel::JunctionRailObject> railObj)
	{
		const auto &coord = railObj->GetPosition();

		m_pclCommandProcessor->Submit(
			new PanelDocumentCommand{
				*m_pclDocument,
				"Insert turnout object",
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
				
		auto contentPanel = new wxPanel(this);		

		auto toolPalette = new wxScrolledWindow(contentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);

		toolPalette->SetScrollRate(0, 8);

		auto paletteSizer = new wxBoxSizer(wxVERTICAL);
		auto toolGrid = new wxGridSizer(0, 2, 0, 0);

		paletteSizer->Add(toolGrid, 0, wxALIGN_TOP);
		toolPalette->SetSizer(paletteSizer);

		m_pclMapCanvas = new LitePanel::Gui::PanelEditorCanvas(contentPanel);

		auto contentSizer = new wxBoxSizer(wxHORIZONTAL);
		contentSizer->Add(toolPalette, 0, wxEXPAND);
		contentSizer->Add(m_pclMapCanvas, 1, wxEXPAND);

		contentPanel->SetSizer(contentSizer);

		auto frameSizer = new wxBoxSizer(wxVERTICAL);
		frameSizer->Add(contentPanel, 1, wxEXPAND);

		SetSizer(frameSizer);

		this->InitMenuBar();

		CreateStatusBar();

		SetStatusText("Welcome to LitePanel Editor!");
		
		this->InitTools(toolPalette, toolGrid);

		toolPalette->FitInside();

		const int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X); 

		const int gridWidth = toolGrid->GetMinSize().x;

		toolPalette->SetMinSize(wxSize(gridWidth, -1));

		toolPalette->Bind(
			wxEVT_SIZE,
			[toolPalette, contentPanel, gridWidth, scrollbarWidth]
			(wxSizeEvent &event)
			{
				event.Skip();

				//after Size is processed, check size for the toolbar and increase it or shrink for the scroolbar
				toolPalette->CallAfter(
					[toolPalette, contentPanel, gridWidth, scrollbarWidth]()
					{
						const bool needsScroll = toolPalette->GetVirtualSize().y > toolPalette->GetClientSize().y;
						const int requiredWidth = gridWidth + (needsScroll ? scrollbarWidth : 0);

						if (toolPalette->GetMinSize().x != requiredWidth)
						{
							toolPalette->SetMinSize(wxSize(requiredWidth, -1));
							contentPanel->Layout();
						}
					}
				);
			}
		);

		contentPanel->Layout();
		Layout();
		
		Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
		Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);		
		m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_LEFT_CLICK, &MainFrame::OnTileLeftClick, this, wxID_ANY);
		m_pclMapCanvas->Bind(LitePanel::Gui::EVT_TILE_UNDER_MOUSE_CHANGED, &MainFrame::OnTileUnderMouseChanged, this, wxID_ANY);

		LitePanel::Render::SetColorStyle(LitePanel::Render::ColorStyles::LIGHT);
	}

	void MainFrame::InitMenuBar()
	{
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

		SetMenuBar(menuBar);
	}

	void MainFrame::InitTools(wxWindow *toolPalette, wxSizer *toolGrid)
	{
		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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



		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		//toolBarLeft1->AddSeparator();
		//toolBarLeft2->AddSeparator();

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
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

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_right_curve_090_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::NORTH,
					LitePanel::SimpleRailTypes::CURVE_RIGHT,
					LitePanel::BlockSplitTypes::BOTTOM
				));
			},
			"Right curve track section",
			"Creates a right curve track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_left_curve_090_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::NORTH,
					LitePanel::SimpleRailTypes::CURVE_LEFT,
					LitePanel::BlockSplitTypes::BOTTOM
				));
			},
			"Left curve track section",
			"Creates a left curve track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_left_curve_270_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::SOUTH,
					LitePanel::SimpleRailTypes::CURVE_LEFT,
					LitePanel::BlockSplitTypes::TOP
				));
			},
			"Left curve track section with top block split",
			"Creates a left curve track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_right_curve_270_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::SOUTH,
					LitePanel::SimpleRailTypes::CURVE_RIGHT,
					LitePanel::BlockSplitTypes::TOP
				));
			},
			"Right curve track section with top block split",
			"Creates a right curve track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track terminal",
			wxBITMAP(rail_terminal_straight_180_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::EAST,
					LitePanel::SimpleRailTypes::STRAIGHT,
					LitePanel::BlockSplitTypes::RIGHT
				));
			},
			"Straight track section with block split",
			"Creates a block split track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_terminal_straight_000_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertRailCmd(std::make_unique<LitePanel::SimpleRailObject>(
					coord,
					LitePanel::ObjectAngles::EAST,
					LitePanel::SimpleRailTypes::STRAIGHT,
					LitePanel::BlockSplitTypes::LEFT
				));
			},
			"Straight track section with block split",
			"Creates a block split track section"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_left_turnout_000_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertJunctionRailCmd(std::make_unique<LitePanel::JunctionRailObject>(
					coord,
					LitePanel::ObjectAngles::EAST,
					LitePanel::JunctionTypes::LEFT_TURNOUT					
				));
			},
			"Left turnout",
			"Creates a left turnout"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_right_turnout_180_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertJunctionRailCmd(std::make_unique<LitePanel::JunctionRailObject>(
					coord,
					LitePanel::ObjectAngles::WEST,
					LitePanel::JunctionTypes::RIGHT_TURNOUT
				));
			},
			"Right turnout",
			"Creates a right turnout"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_left_turnout_180_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertJunctionRailCmd(std::make_unique<LitePanel::JunctionRailObject>(
					coord,
					LitePanel::ObjectAngles::WEST,
					LitePanel::JunctionTypes::LEFT_TURNOUT
				));
			},
			"Left turnout",
			"Creates a left turnout"
		);

		m_clToolManager.AddTool(
			toolPalette,
			toolGrid,
			"track",
			wxBITMAP(rail_right_turnout_000_icon),
			[this](const LitePanel::TileCoord_t &coord) -> void
			{
				ExecuteInsertJunctionRailCmd(std::make_unique<LitePanel::JunctionRailObject>(
					coord,
					LitePanel::ObjectAngles::EAST,
					LitePanel::JunctionTypes::RIGHT_TURNOUT
				));
			},
			"Right turnout",
			"Creates a right turnout"
		);
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

	void MainFrame::OnTileLeftClick(LitePanel::Gui::TileEvent &event)
	{
		auto tilePos = event.GetTilePosition();

		if (!tilePos)
		{
			//should never happen
			this->SetStatusText("No tile");

			return;
		}

		auto proc = m_clToolManager.TryGetSelectedToolProc();
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
				auto obj = std::make_unique<LitePanel::QuadObject>(position.value(), LitePanel::Render::GetCurrentColorStyle().m_tTileHighLight);

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