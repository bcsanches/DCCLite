// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "GLCanvas.h"


namespace OpenGLState
{	wxGLContext *g_pclContext = nullptr;

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
