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

#include <vector>

#include <fmt/format.h>

#include "imgui.h"

#include "Util.h"

#include "EditorWidget.h"

namespace dcclite::panel_editor
{
	class ConsoleWidget: public EditorWidget
	{
		public:
			ConsoleWidget();
			virtual ~ConsoleWidget();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(ConsoleWidget);

			void Display() override;
			void Update() override;			

			template <typename... Args>
			inline void AddLog(fmt::format_string<Args...> s, Args&&... args)
			{
				this->AddLogImpl(fmt::format(s, std::forward<Args>(args)...));
			}

		private:
			void AddLogImpl(std::string log);

			void ClearLog();
			void ClearHistory();

			int OnTextEdit(ImGuiInputTextCallbackData *data);

			friend int TextEditCallbackStub(ImGuiInputTextCallbackData *data);

			void ExecuteCommand(const char *cmd);

		private:
			char m_arInputBuffer[256];

			std::vector<std::string>	m_vecEntries;
			std::vector<std::string>	m_vecHistory;

			long long                   m_iHistoryPos = -1;    // -1: new line, 0..History.Size-1 browsing history.

			ImGuiTextFilter       m_clFilter;

			bool                  m_fAutoScroll = true;
			bool                  m_fScrollToBottom = false;
	};
}