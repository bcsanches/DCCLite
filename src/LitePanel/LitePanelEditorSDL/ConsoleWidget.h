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

#include <functional>
#include <vector>

#include <fmt/format.h>

#include "imgui.h"

#include "Util.h"

#include "EditorWidget.h"
#include "Object.h"

namespace dcclite::panel_editor
{
	class ConsoleWidget;	

	class ConsoleCmd: public dcclite::IObject
	{
		protected:
			ConsoleCmd(RName name);

		public:			
			virtual void Execute(ConsoleWidget &owner, unsigned argc, const std::string_view *argv) = 0;

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(ConsoleCmd);

			const char *GetTypeName() const noexcept override
			{
				return "ConsoleCmd";
			}

		private:			
	};

	typedef std::function<void (ConsoleWidget &owner, unsigned argc, const std::string_view *argv)> ConsoleCmdFunc_t;

	class SimpleConsoleCmd : public ConsoleCmd
	{
		public:
			SimpleConsoleCmd(RName name, ConsoleCmdFunc_t func) :
				ConsoleCmd{ name },
				m_pfnFunc{ func }
			{
				if (!func)
				{
					throw std::invalid_argument("[SimpleConsoleCmd] func cannot be null");
				}
			}

			void Execute(ConsoleWidget &owner, unsigned argc, const std::string_view *argv) override
			{
				m_pfnFunc(owner, argc, argv);
			}

		private:
			ConsoleCmdFunc_t m_pfnFunc;
	};

	class ConsoleWidget: public EditorWidget
	{
		public:
			ConsoleWidget();
			virtual ~ConsoleWidget();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(ConsoleWidget);

			void Display() override;
			void Update() override;						

			inline void RegisterCommand(std::unique_ptr<ConsoleCmd> cmd)
			{
				m_clCommands.AddChild(std::move(cmd));
			}

			inline void RegisterCommand(RName name, ConsoleCmdFunc_t func)
			{
				m_clCommands.AddChild(std::make_unique<SimpleConsoleCmd>(name, func));
			}

		private:
			template <typename... Args>
			inline void AddLog(fmt::format_string<Args...> s, Args&&... args)
			{
				this->AddLogImpl(fmt::format(s, std::forward<Args>(args)...));
			}

			void AddLogImpl(std::string log);

			void ClearLog();
			void ClearHistory();

			int OnTextEdit(ImGuiInputTextCallbackData *data);

			friend int TextEditCallbackStub(ImGuiInputTextCallbackData *data);

			void ExecuteCommand(const char *cmd);

			friend class LogSink;
			friend class HelpCommand;
			friend class ClearCommand;
			friend class ClearHistoryCommand;

		private:
			char m_arInputBuffer[256];

			std::vector<std::string>	m_vecEntries;
			std::vector<std::string>	m_vecHistory;

			dcclite::FolderObject		m_clCommands;

			long long                   m_iHistoryPos = -1;    // -1: new line, 0..History.Size-1 browsing history.

			ImGuiTextFilter				m_clFilter;

			std::shared_ptr<LogSink>	m_spLogSink;

			bool                  m_fAutoScroll = true;
			bool                  m_fScrollToBottom = false;
	};
}