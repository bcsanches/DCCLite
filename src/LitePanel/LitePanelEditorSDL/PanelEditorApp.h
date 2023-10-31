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

#include "imgui.h"

#include <Util.h>

#include "ConsoleWidget.h"
#include "StatusBarWidget.h"
#include "ToolBarWidget.h"

namespace dcclite::panel_editor
{
	class PanelEditorApp
	{
		public:
			PanelEditorApp();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(PanelEditorApp);

			bool Display();
			
			void Run();

		private:
			dcclite::panel_editor::ConsoleWidget	m_wConsole;
			dcclite::panel_editor::StatusBarWidget	m_wStatusBar;
			dcclite::panel_editor::ToolBarWidget	m_wToolBar;

			//
			//
			//Gui states
			// 
			ImVec2	m_vec2ViewportSize = { 0, 0 };

			bool	m_fShowAbout = false;
			bool	m_fShowDemo = false;
			bool	m_fShowMetrics = false;
			bool	m_fShowDebugLog = false;
			bool	m_fShowIdStackTool = false;

			bool	m_fKeepRunning = true;
	};
}
