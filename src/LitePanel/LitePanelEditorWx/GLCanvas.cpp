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

#include <algorithm>

#include <backends/imgui_impl_opengl3.h>
#include "imgui.h"

namespace LitePanel::Gui
{
	OGLCanvas::OGLCanvas(wxWindow *parent, int id):
		wxGLCanvas(parent, id, [] {
			static const int attributes[] =
			{
				WX_GL_RGBA,
				WX_GL_DOUBLEBUFFER,
				WX_GL_DEPTH_SIZE, 16,
				WX_GL_MAJOR_VERSION, 3,
				WX_GL_MINOR_VERSION, 0,
				WX_GL_PROFILE, WX_GL_COMPAT_PROFILE,
				0
			};
			return attributes;
		}())
	{
		m_upContext = std::make_unique<wxGLContext>(this);

		Bind(wxEVT_PAINT, &OGLCanvas::OnPaint, this);
		Bind(wxEVT_ERASE_BACKGROUND, &OGLCanvas::OnEraseBackground, this);
	}

	OGLCanvas::~OGLCanvas()
	{
		if (m_fInitialized && m_upContext)
		{
			m_upContext->SetCurrent(*this);
			ImGui_ImplOpenGL3_Shutdown();
			ImGui::DestroyContext();
		}
	}

	bool OGLCanvas::InitGL()
	{
		if (!m_upContext || !m_upContext->IsOK() || !m_upContext->SetCurrent(*this))
			return false;

		glViewport(0, 0, GetSize().x, GetSize().y);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.DisplaySize = ImVec2(static_cast<float>(GetClientSize().x), static_cast<float>(GetClientSize().y));
		ImGui::StyleColorsClassic();

		if (!ImGui_ImplOpenGL3_Init("#version 130"))
		{
			ImGui::DestroyContext();
			return false;
		}

		m_fInitialized = true;
		return true;
	}

	void OGLCanvas::OnPaint(wxPaintEvent &e)
	{
		wxPaintDC paintDC(this);

		if (!IsShown() || !m_upContext || !m_upContext->IsOK())
			return;

		if (!m_upContext->SetCurrent(*this))
			return;

		if (!m_fInitialized && !InitGL())
			return;

		const auto clientSize = GetClientSize();
		if (clientSize.x <= 0 || clientSize.y <= 0)
			return;

		ImGuiIO &io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(clientSize.x), static_cast<float>(clientSize.y));
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.DeltaTime = 1.0f / 60.0f;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		// The wxWidgets application continues to own all input and editor tools.
		// ImGui is used here only as the shared rendering layer.
		OnDraw();

		ImGui::Render();
		glViewport(0, 0, clientSize.x, clientSize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SwapBuffers();
	}

	void OGLCanvas::OnEraseBackground(wxEraseEvent &e)
	{
		// Avoid background erasure flicker on the GL canvas.
	}
}
