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
#include "Document.h"
#include "DocumentWidget.h"
#include "KeyBindingManager.h"
#include "StatusBarWidget.h"
#include "ToolBarWidget.h"

namespace dcclite::PanelEditor
{	
	class AppTask
	{
		public:
			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(AppTask);

			virtual ~AppTask() = default;

			virtual bool Display() = 0;

		protected:
			AppTask() = default;			
	};

	class AppTask;

	class PanelEditorApp
	{
		public:
			PanelEditorApp();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(PanelEditorApp);

			bool Display();
			
			void Run();			

			inline void Bind(const char *cmd, SDL_Scancode key, uint32_t keyMod = 0)
			{
				m_clBindings.Bind(cmd, key, keyMod);
			}

			void HandleEvent(const SDL_KeyboardEvent &key);

			inline ToolBarWidget &GetToolBar() noexcept
			{
				return m_wToolBar;
			}
	
		private:
			void NewFile();

			void PushTask(std::unique_ptr<AppTask> task);

		private:
			dcclite::PanelEditor::ConsoleWidget		m_wConsole;
			dcclite::PanelEditor::StatusBarWidget	m_wStatusBar;
			dcclite::PanelEditor::ToolBarWidget		m_wToolBar;
			dcclite::PanelEditor::DocumentWidget	m_wDocumentWidget;

			std::unique_ptr<AppTask>				m_upTask;

			KeyBindingManager						m_clBindings;

			Document								m_clDocument;

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
