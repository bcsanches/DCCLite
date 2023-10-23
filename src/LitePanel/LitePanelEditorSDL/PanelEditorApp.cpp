// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "PanelEditorApp.h"

#include "imgui_internal.h"

static void ImGuiDemoFunc()
{
	// Our state
	static bool show_demo_window = true;
	static bool show_another_window = false;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");								// Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");					// Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);			// Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);				// Edit 1 float using a slider from 0.0f to 1.0f		

		if (ImGui::Button("Button"))								// Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGuiIO &io = ImGui::GetIO();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

static void ShowAboutWindow(bool *p_open)
{
	if (ImGui::Begin("About Lite Panel", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text("Lite Panel %s", DCCLITE_VERSION);
		ImGui::Separator();
		ImGui::Text("By Bruno C. Sanches");
		ImGui::Text("Licensed under the MIT License, see LICENSE for more information.");
	}

	ImGui::End();
}


namespace dcclite::panel_editor
{
	PanelEditorApp::PanelEditorApp()
	{
		//empty
	}


	bool PanelEditorApp::Display()
	{
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

		//disable padding, rouding and border
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("MainWindow", nullptr, window_flags);

		//restore padding, rouding and border
		ImGui::PopStyleVar(3);

		// Submit the DockSpace
		ImGuiIO &io = ImGui::GetIO();

		ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");		

		ImVec2 currentViewPortSize = viewport->WorkSize;

		ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
		if (!ImGui::DockBuilderGetNode(dockspaceID) || ((currentViewPortSize.x != m_vec2ViewportSize.x) || (currentViewPortSize.y != m_vec2ViewportSize.y)))
		{
			m_vec2ViewportSize = currentViewPortSize;

			ImGui::DockBuilderRemoveNode(dockspaceID);
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);
			ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

			auto dock_down_height = (32.0f / currentViewPortSize.y) + 0.001f;
			auto leftToolBarWidth = (64.0f / currentViewPortSize.x) + 0.001f;

			ImGuiID dock_main_id = dockspaceID;
			auto bottomNodeId = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, dock_down_height, nullptr, &dock_main_id);
			auto leftToolBarNodeId = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, leftToolBarWidth, nullptr, &dock_main_id);

			auto workAreaNodeId = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);

			{
				ImGui::DockBuilderDockWindow("LToolBar", leftToolBarNodeId);
				auto leftNode = ImGui::DockBuilderGetNode(leftToolBarNodeId);

				leftNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking;
			}

			ImGui::DockBuilderDockWindow("StatusBar", bottomNodeId);
#if 1
			{
				auto downNode = ImGui::DockBuilderGetNode(bottomNodeId);
				downNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoResize;
			}
#endif

			ImGui::DockBuilderDockWindow("Console", workAreaNodeId);

			{
				ImGui::DockBuilderDockWindow("WorkArea", dock_main_id);
				auto mainNode = ImGui::DockBuilderGetNode(dock_main_id);
				mainNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking;
			}

			ImGui::DockBuilderFinish(dock_main_id);
		}

		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);		

		bool keepRunning = true;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit", "Alt+F4"))
					keepRunning = false;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("Show Demo Window", "", &m_fShowDemo);
				ImGui::MenuItem("Show Metrics Window", "", &m_fShowMetrics);
				ImGui::MenuItem("Show Debug Log Window", "", &m_fShowDebugLog);
				ImGui::MenuItem("Show Id Stack Tool Window", "", &m_fShowIdStackTool);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("About", nullptr, &m_fShowAbout);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();

		//https://github.com/ocornut/imgui/issues/2583

		m_wStatusBar.Display();
		m_wToolBar.Display();

		if (ImGui::Begin("WorkArea", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
			{
				static bool open = true;
				if (ImGui::BeginTabItem("Untitled Panel", &open, 0))
				{
					ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
					ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

					ImGui::Text("Cool! work area");

					auto drawList = ImGui::GetWindowDrawList();

					ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 0, 255, 255));

					drawList->AddLine(canvas_p0, canvas_p0 + canvas_sz, col_b);

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Untitled Panel 2", nullptr, 0))
				{
					auto drawList = ImGui::GetWindowDrawList();

					ImGui::Text("Gradients");
					ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
					{
						ImVec2 p0 = ImGui::GetCursorScreenPos();
						ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);
						ImU32 col_a = ImGui::GetColorU32(IM_COL32(0, 0, 0, 255));
						ImU32 col_b = ImGui::GetColorU32(IM_COL32(255, 255, 255, 255));
						drawList->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
						ImGui::InvisibleButton("##gradient1", gradient_size);
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		m_wConsole.Display();

		if (m_fShowAbout)
			ShowAboutWindow(&m_fShowAbout);

		if (m_fShowDemo)
			ImGui::ShowDemoWindow(&m_fShowDemo);

		if (m_fShowMetrics)
			ImGui::ShowMetricsWindow(&m_fShowMetrics);

		if (m_fShowDebugLog)
			ImGui::ShowDebugLogWindow(&m_fShowDebugLog);

		if (m_fShowIdStackTool)
			ImGui::ShowIDStackToolWindow(&m_fShowIdStackTool);

		return keepRunning;
	}

	void PanelEditorApp::Run()
	{

	}
}
