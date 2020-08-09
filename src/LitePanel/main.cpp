// wxWidgets "Hello World" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/glcanvas.h>
#endif

#include <cstdint>

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

template <typename T>
struct Point
{
	T m_tX = {0}, m_tY = {0};

	Point() = default;

	Point(T x, T y):
		m_tX(x),
		m_tY(y)
	{
		//empty
	}

	const Point operator/(T num) const
	{
		return Point(m_tX / num, m_tY / num);
	}

	const Point operator*(T num) const
	{
		return Point{m_tX * num, m_tY * num};
	}

	const Point operator+(const Point &rhs) const
	{
		return Point(m_tX + rhs.m_tX, m_tY + rhs.m_tY);
	}

	const Point operator-(const Point &rhs) const
	{
		return Point(m_tX - rhs.m_tX, m_tY - rhs.m_tY);
	}

	const Point &operator+=(const Point &rhs)
	{
		m_tX += rhs.m_tX;
		m_tY += rhs.m_tY;

		return *this;
	}
};

typedef Point<int_fast32_t> IntPoint_t;

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

		void OnMouseMiddleDown(wxMouseEvent &event);
		void OnMouseMiddleUp(wxMouseEvent &event);
		void OnMouseMove(wxMouseEvent &event);

	private:
		IntPoint_t m_tOrigin;
		IntPoint_t m_tTileMapSize;
		
		IntPoint_t m_tMoveStartPos;
};

TestCanvas::TestCanvas(wxWindow *parent, int id):
	OGLCanvas{parent, id},
	m_tTileMapSize{64, 64}
{
	Bind(wxEVT_MIDDLE_DOWN, &TestCanvas::OnMouseMiddleDown, this);
	Bind(wxEVT_MIDDLE_UP, &TestCanvas::OnMouseMiddleUp, this);
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

	IntPoint_t tileOrigin = m_tOrigin / TILE_SIZE;
	IntPoint_t drawOffset = IntPoint_t{m_tOrigin.m_tX % TILE_SIZE, m_tOrigin.m_tY % TILE_SIZE};

	//how many tiles we can fit on screen?
	IntPoint_t screenTilesSize{
		std::max(GetSize().x / TILE_SIZE, m_tTileMapSize.m_tX),
		std::max(GetSize().y / TILE_SIZE, m_tTileMapSize.m_tY)
	};		

	const IntPoint_t worldMax{m_tTileMapSize * TILE_SIZE};

	const IntPoint_t virtualScreenMax{m_tOrigin.m_tX + size.x, m_tOrigin.m_tY + size.y};
	const IntPoint_t visibleLimit = IntPoint_t{
		std::min(worldMax.m_tX, virtualScreenMax.m_tX) - m_tOrigin.m_tX,
		std::min(worldMax.m_tY, virtualScreenMax.m_tY) - m_tOrigin.m_tY
	};

	glBegin(GL_LINES);

	for (int i = 0; i <= screenTilesSize.m_tX; ++i)
	{
		IntPoint_t tilePos = IntPoint_t{tileOrigin.m_tX + i, tileOrigin.m_tY};

		if(tilePos.m_tX > m_tTileMapSize.m_tX)
			break;

		IntPoint_t worldPos = tilePos * TILE_SIZE;
		IntPoint_t screenOrigin = worldPos - m_tOrigin;

		glVertex2i(screenOrigin.m_tX, 0);
		glVertex2i(screenOrigin.m_tX, visibleLimit.m_tY);
	}

	for (int i = 0; i <= screenTilesSize.m_tY; ++i)
	{
		IntPoint_t tilePos = IntPoint_t{ tileOrigin.m_tX, tileOrigin.m_tY + i};

		if (tilePos.m_tY > m_tTileMapSize.m_tY)
			break;

		IntPoint_t worldPos = tilePos * TILE_SIZE;
		IntPoint_t screenOrigin = worldPos - m_tOrigin;

		glVertex2i(0, screenOrigin.m_tY);
		glVertex2i(visibleLimit.m_tX, screenOrigin.m_tY);
	}

#if 0
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
#endif

	glEnd();

	this->SwapBuffers();
}

void TestCanvas::OnMouseMiddleDown(wxMouseEvent &event)
{
	event.Skip();

	this->Bind(wxEVT_MOTION, &TestCanvas::OnMouseMove, this);

	m_tMoveStartPos = IntPoint_t{event.GetX(), event.GetY()};	
}

void TestCanvas::OnMouseMove(wxMouseEvent &event)
{
	event.Skip();

	auto currentPos = IntPoint_t{ event.GetX(), event.GetY() };

	m_tOrigin += currentPos - m_tMoveStartPos;
	m_tOrigin.m_tX = std::max(0, m_tOrigin.m_tX);
	m_tOrigin.m_tY = std::max(0, m_tOrigin.m_tY);

	m_tMoveStartPos = currentPos;

	this->Refresh();
}


void TestCanvas::OnMouseMiddleUp(wxMouseEvent &event)
{
	event.Skip();

	this->Unbind(wxEVT_MOTION, &TestCanvas::OnMouseMove, this);
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
