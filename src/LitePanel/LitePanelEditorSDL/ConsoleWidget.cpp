// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ConsoleWidget.h"

#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include "imgui_internal.h"

#include "Log.h"
#include "LogUtils.h"

namespace dcclite::PanelEditor
{
    class LogSink : public spdlog::sinks::base_sink<std::mutex>
    {
        public:
            LogSink(ConsoleWidget &owner) :
                m_rclConsole{owner}
            {
                //empty
            }

            void sink_it_(const spdlog::details::log_msg &msg) override
            {
                // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
                // msg.raw contains pre formatted log

                // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
                spdlog::memory_buf_t formatted;
                spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);

                m_rclConsole.AddLog(fmt::to_string(formatted));                
            }

            void flush_() override
            {
                
                //std::cout << std::flush;
            }

        private:
            ConsoleWidget &m_rclConsole;
    };

    ConsoleCmd::ConsoleCmd(RName name) :
        IObject{ name }
    {
        //empty
    }

    ConsoleWidget::ConsoleWidget():
        m_clCommands{RName::Create("ConsoleCommands")}
    {
        m_arInputBuffer[0] = '\0';        
        
        m_spLogSink = std::make_shared<LogSink>(*this);
        m_spLogSink->set_pattern("[%T] [%^-%L-%$] [T %t] %v");

        auto &sinks = dcclite::LogGetDefault()->sinks();
        sinks.push_back(m_spLogSink);

        this->RegisterCommand(RName{ "help" }, [](ConsoleCmdParams &params)
            {
                if (params.m_uArgc > 1)
                {
                    dcclite::Log::Error("[HelpCommand] No arguments expected.");

                    return;
                }

                auto enumerator = params.m_rclConsole.m_clCommands.GetEnumerator();

                while (enumerator.MoveNext())
                {
                    auto cmd = enumerator.GetCurrent();

                    params.m_rclConsole.AddLog("\t{}", cmd->GetName().GetData());
                }
            }
        );

        this->RegisterCommand(RName{ "Console.Clear" }, [](ConsoleCmdParams &params)
            {
                if (params.m_uArgc > 1)
                {
                    dcclite::Log::Error("[ClearCommand] No arguments expected.");

                    return;
                }

                params.m_rclConsole.ClearLog();
            }
        );

        this->RegisterCommand(RName{ "Console.ClearHistory"}, [](ConsoleCmdParams &params)
            {
                if (params.m_uArgc > 1)
                {
                    dcclite::Log::Error("[ClearHistoryCommand] No arguments expected.");

                    return;
                }

                params.m_rclConsole.ClearHistory();
            }
        );        

        dcclite::Log::Info("[ConsoleWidget::ConsoleWidget] Created and sink registered");
    }

	ConsoleWidget::~ConsoleWidget()
	{
        auto &sinks = dcclite::LogGetDefault()->sinks();

        auto it = std::find_if(sinks.begin(), sinks.end(), [this](spdlog::sink_ptr &ptr)
            {
                return ptr == m_spLogSink;
            }
        );
     
        sinks.erase(it);
	}

    void ConsoleWidget::AddLogImpl(std::string log)
    {
        m_vecEntries.push_back(std::move(log));
    }

    void ConsoleWidget::ClearLog()
    {
        m_vecEntries.clear();
    }

    void ConsoleWidget::ClearHistory()
    {
        m_vecHistory.clear();
    }

    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data)
    {
        ConsoleWidget *console = (ConsoleWidget *)data->UserData;
        return console->OnTextEdit(data);
    }

    int ConsoleWidget::OnTextEdit(ImGuiInputTextCallbackData *data)
    {
        switch (data->EventFlag)
        {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char *word_end = data->Buf + data->CursorPos;
                const char *word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char *> candidates;

                auto enumerator = m_clCommands.GetEnumerator();

                while (enumerator.MoveNext())
                {
                    auto cmd = enumerator.GetCurrent();

                    auto cmdName = cmd->GetNameData();
                    if(cmdName.compare(0, (word_end - word_start), word_start) == 0)
                        candidates.push_back(cmdName.data());
                }

#if 0
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);
#endif

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- {}\n", candidates[i]);
                }

                break;
            }

            case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const auto prev_history_pos = m_iHistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (m_iHistoryPos == -1)
                        m_iHistoryPos = m_vecHistory.size() - 1;
                    else if (m_iHistoryPos > 0)
                        m_iHistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (m_iHistoryPos != -1)
                        if (++m_iHistoryPos >= (long long)m_vecHistory.size())
                            m_iHistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != m_iHistoryPos)
                {
                    const char *history_str = (m_iHistoryPos >= 0) ? m_vecHistory[m_iHistoryPos].c_str() : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }

    constexpr auto MAX_CONSOLE_ARGS = 16;

    static inline bool IsConsoleToken(char ch) noexcept
    {
        return ((ch == '.') || ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')));
    }

    static int BuildConsoleArgv(const char *command, std::string_view *args)
    {
        int argc = 0;

        while(*command)
        {
            switch (*command)
            {
                case ' ':
                case '\t':
                    ++command;
                    continue;                

                default:
                    if (!IsConsoleToken(*command))
                        throw std::invalid_argument(fmt::format("[BuildConsoleArgv] Invalid char: {}", *command));

                    const char *start = command;
                    
                    while (*command && IsConsoleToken(*command))
                        ++command;

                    args[argc++] = std::string_view{ start, static_cast<size_t>(command - start)};
                    break;
            }
        }

        return argc;
    }

    void ConsoleWidget::ExecuteCommand(const char *command)
    {
        this->AddLog("> {}", command);    

        std::string_view args[MAX_CONSOLE_ARGS];

        int argc = BuildConsoleArgv(command, args);

        //empty command?
        if (argc <= 0)
            return;

        m_vecHistory.push_back(command);        

        RName cmdName = RName::TryGetName(args[0]);
        if (!cmdName)
        {
CMD_NOT_FOUND:
            dcclite::Log::Error("[ConsoleWidget::ExecuteCommand] Command {} is not registered", args[0]);

            return;
        }

        auto cmdHandler = static_cast<ConsoleCmd *>(m_clCommands.TryGetChild(cmdName));
        if (!cmdHandler) 
        {
            goto CMD_NOT_FOUND;
        }        

        cmdHandler->Execute(ConsoleCmdParams{ *this, static_cast<unsigned>(argc), args });
    }

	void ConsoleWidget::Display()
	{
        //ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Console", nullptr, 0))
        {
            ImGui::End();
            return;
        }        

        if (ImGui::Button("Clear"))
        {
            this->ClearLog();
        }

        ImGui::SameLine();

        bool copy_to_clipboard = ImGui::Button("Copy");

        ImGui::SameLine();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &m_fAutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        m_clFilter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear"))
                    this->ClearLog();

                ImGui::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      ImGuiListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                ImGui::LogToClipboard();
            for (auto const &item : m_vecEntries)
            {
                if (!m_clFilter.PassFilter(item.c_str()))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if (item.find("[error]") != std::string::npos) 
                { 
                    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); 
                    has_color = true; 
                }
                else if (item.compare(0, 2, "# ") == 0) 
                { 
                    color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); 
                    has_color = true; 
                }

                if (has_color)
                    ImGui::PushStyleColor(ImGuiCol_Text, color);

                ImGui::TextUnformatted(item.c_str());

                if (has_color)
                    ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (m_fScrollToBottom || (m_fAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);

            m_fScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();

#if 1
        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("Input", m_arInputBuffer, IM_ARRAYSIZE(m_arInputBuffer), input_text_flags, &TextEditCallbackStub, (void *)this))
        {
            this->ExecuteCommand(m_arInputBuffer);
            m_arInputBuffer[0] = '\0';
            
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
#endif

        ImGui::End();
	}    


	void ConsoleWidget::Update()
	{

	}
}