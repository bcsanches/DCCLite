// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/glcanvas.h>
#endif

class OGLCanvas: public wxGLCanvas
{
	public:
		OGLCanvas(wxWindow *parent, int id);
		~OGLCanvas() = default;

	private:
		// Events
		void OnPaint(wxPaintEvent &e);
		void OnEraseBackground(wxEraseEvent &e);
};

OGLCanvas::OGLCanvas(wxWindow *parent, int id):
	wxGLCanvas(parent, id) 
{
	// Bind events
	Bind(wxEVT_PAINT, &OGLCanvas::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &OGLCanvas::OnEraseBackground, this);
}

void OGLCanvas::OnPaint(wxPaintEvent &e)
{

}

void OGLCanvas::OnEraseBackground(wxEraseEvent &e)
{
	//empty
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
	menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
		"Help string shown in status bar for this menu item");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to wxWidgets!");

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
