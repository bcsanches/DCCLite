// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "Project.h"
#include "fmt/format.h"

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
		void OnOpen(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);

	private:
		Project m_clProject;
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

MainFrame::MainFrame()
	: wxFrame(NULL, wxID_ANY, "Lite Panel")
{
	wxMenu* menuFile = new wxMenu;

	menuFile->Append(wxID_NEW, "&New...\tCtrl-N",
		"Create new project");	
	menuFile->Append(wxID_OPEN, "&Open...\tCtrl-O",
		"Open existing project");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to LiteWiring!");

	Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}

void MainFrame::OnOpen(wxCommandEvent& event)
{	
	wxString path = wxFileSelector(
		"Select the file to load",
		wxEmptyString, 
		wxEmptyString,
		wxEmptyString,
		wxString::Format
		(
			"All files (%s)|%s|Wiring json files (*.json)|*.json",
			wxFileSelectorDefaultWildcardStr,
			wxFileSelectorDefaultWildcardStr
		),
		wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_PREVIEW | wxFD_NO_FOLLOW | wxFD_SHOW_HIDDEN | wxFD_FILE_MUST_EXIST,
		this
	);

	if (!path)
		return;
	
	auto strPath = path.ToStdString();
	m_clProject.Load(path.ToStdString());

	SetStatusText(fmt::format("Loaded {}", strPath));
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


