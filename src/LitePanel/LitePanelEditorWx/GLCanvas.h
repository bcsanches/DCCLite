// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <memory>

#include <wx/wx.h>
#include <wx/glcanvas.h>

class wxGLContext;

namespace LitePanel::Gui
{
	class OGLCanvas: public wxGLCanvas
	{
		public:
		OGLCanvas(wxWindow *parent, int id = -1);
		~OGLCanvas() override;

		protected:
		virtual void OnDraw() = 0;

		private:
		void OnPaint(wxPaintEvent &e);
		void OnEraseBackground(wxEraseEvent &e);

		bool InitGL();

		private:
		std::unique_ptr<wxGLContext> m_upContext;
		bool m_fInitialized = false;
	};
}
