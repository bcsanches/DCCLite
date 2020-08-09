// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/glcanvas.h>
#endif

namespace OpenGLState
{
	wxGLContext *g_pclContext = nullptr;

	void SetCurrent(wxGLCanvas &win)
	{
		if(!win.IsShown())
			return;

		if (!g_pclContext)
		{
			g_pclContext = new wxGLContext(&win);
			g_pclContext->SetCurrent(win);
		}
	}
}


class OGLCanvas: public wxGLCanvas
{
	public:
		OGLCanvas(wxWindow *parent, int id = -1);
		~OGLCanvas() = default;

	protected:
		virtual void OnDraw() = 0;

	private:
		// Events
		void OnPaint(wxPaintEvent &e);
		void OnEraseBackground(wxEraseEvent &e);

		void InitGL();

	private:		
		bool m_fInitialized = false;
		
};

OGLCanvas::OGLCanvas(wxWindow *parent, int id):
	wxGLCanvas(parent, id) 
{
	// Bind events
	Bind(wxEVT_PAINT, &OGLCanvas::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &OGLCanvas::OnEraseBackground, this);
}

void OGLCanvas::InitGL()
{
	glViewport(0, 0, GetSize().x, GetSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glEnable(GL_ALPHA_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	m_fInitialized = true;
}

void OGLCanvas::OnPaint(wxPaintEvent &e)
{
	if(!this->IsShown())
		return;

	OpenGLState::SetCurrent(*this);

	if(!m_fInitialized)
		this->InitGL();

	this->OnDraw();
}

void OGLCanvas::OnEraseBackground(wxEraseEvent &e)
{
	//empty
}

class TestCanvas: public OGLCanvas
{
	public:
		TestCanvas(wxWindow *parent, int id = -1);

	protected:
		void OnDraw() override;
};

TestCanvas::TestCanvas(wxWindow *parent, int id):
	OGLCanvas(parent, id)
{
	//empty
}

void TestCanvas::OnDraw()
{
	// Setup the viewport
	glViewport(0, 0, GetSize().x, GetSize().y);

	// Setup the screen projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, GetSize().x, GetSize().y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	const auto TILE_SIZE = 16;
	const auto size = this->GetSize();

	glBegin(GL_LINES);

	for (int i = 0; i < size.x; i += TILE_SIZE)
	{		
		glVertex2i(i, 0);
		glVertex2i(i, size.y);		
	}

	for (int i = 0; i < size.y; i += TILE_SIZE)
	{		
		glVertex2i(0, i);
		glVertex2i(size.x, i);		
	}

	glEnd();

	this->SwapBuffers();
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

	auto oglCanvas = new TestCanvas(this);

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
