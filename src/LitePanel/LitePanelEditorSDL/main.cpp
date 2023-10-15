// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <SDL3/SDL.h>

#include <ConsoleUtils.h>
#include <Log.h>
#include <LogUtils.h>

#include <spdlog/logger.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"

constexpr auto BUILD_NUM = DCCLITE_VERSION;

static bool fExitRequested = false;

ImVec4 g_clClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float *)&g_clClearColor); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
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

static void DisplayMainWindow()
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

	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;	
	if (!ImGui::DockBuilderGetNode(dockspaceID)) 
	{
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_None);
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

		ImGuiID dock_main_id = dockspaceID;
		//ImGuiID dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.05f, nullptr, &dock_main_id);
		//ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
		ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.045f, nullptr, &dock_main_id);		
		ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.045f, nullptr, &dock_main_id);		

		ImGuiID work_area_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);
		//ImGuiID dock_down_right_id = ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

		//ImGui::DockBuilderDockWindow("Actions", dock_up_id);
		//ImGui::DockBuilderDockWindow("Hierarchy", dock_right_id);
		{
			ImGui::DockBuilderDockWindow("LToolBar", dock_left_id);
			auto leftNode = ImGui::DockBuilderGetNode(dock_left_id);

			leftNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking;
		}

		ImGui::DockBuilderDockWindow("StatusBar", dock_down_id);
		//ImGui::DockBuilderSetNodeSize(dock_down_id, ImVec2{ viewport->WorkSize.x, 32 });

#if 1
		{
			auto downNode = ImGui::DockBuilderGetNode(dock_down_id);
			downNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking;
		}
#endif

		ImGui::DockBuilderDockWindow("Console", work_area_bottom);

		{
			ImGui::DockBuilderDockWindow("WorkArea", dock_main_id);
			auto mainNode = ImGui::DockBuilderGetNode(dock_main_id);
			mainNode->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoUndocking;
		}

		// Disable tab bar for custom toolbar
		//ImGuiDockNode *node = ImGui::DockBuilderGetNode(dock_up_id);
		//node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		ImGui::DockBuilderFinish(dock_main_id);
	}	
	
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	static bool show_about = false;
	static bool show_demo = false;
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit", "Alt+F4"))
				fExitRequested = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::EndMenu();
		}		

		if (ImGui::BeginMenu("Options"))
		{									
			ImGui::MenuItem("Show Demo Window", "", &show_demo);
			
			ImGui::EndMenu();
		}		

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("About", nullptr, &show_about);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();

	//https://github.com/ocornut/imgui/issues/2583
	
	//ImGui::SetNextWindowSize(ImVec2{ viewport->WorkSize.x, ImGui::GetTextLineHeightWithSpacing() });
	if (ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::Text("Status Bar");
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();

		ImGui::Text("Status 2");
	}
	ImGui::End();		

	//ImGui::SetNextWindowDockID(dock_left_id, ImGuiCond_Always);
	if (ImGui::Begin("LToolBar", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::SmallButton("X");
		ImGui::SameLine();
		ImGui::SmallButton("Y");

		ImGui::SmallButton("Z");
		ImGui::SameLine();
		ImGui::SmallButton("W");
	}
	ImGui::End();

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

	ImGui::Begin("Console", nullptr, 0);
	ImGui::Text("Cool! A console");
	ImGui::End();

	if (show_about)
		ShowAboutWindow(&show_about);

	if (show_demo)
		ImGui::ShowDemoWindow(&show_demo);
}

int main(int argc, char **argv)
{				
	try
	{ 
		dcclite::LogInit("LitePanelSDL.log");

#ifndef DEBUG
		dcclite::LogGetDefault()->set_level(spdlog::level::trace);
#else
		dcclite::LogGetDefault()->set_level(spdlog::level::trace);
#endif

		dcclite::Log::Info("LitePanelSDL {} {}", BUILD_NUM, __DATE__);		

		dcclite::ConsoleTryMakeNice();

		// Setup SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMEPAD) != 0)
		{
			dcclite::Log::Critical("[LitePanelSDL] Error: SDL_Init(): %s\n", SDL_GetError());
			return -1;
		}

		// Enable native IME.
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		// Create window with SDL_Renderer graphics context
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
		SDL_Window *window = SDL_CreateWindow("LitePanelSDL", 1280, 720, window_flags);
		if (window == nullptr)
		{
			dcclite::Log::Critical("[LitePanelSDL] Error: SDL_CreateWindow(): %s\n", SDL_GetError());
			return -1;
		}

		SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		if (renderer == nullptr)
		{
			dcclite::Log::Critical("[LitePanelSDL] Error: SDL_CreateRenderer(): %s\n", SDL_GetError());			
			return -1;
		}

		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_ShowWindow(window);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO &io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		
		io.IniFilename = nullptr;

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);			

		while (!fExitRequested)
		{
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{				
				ImGui_ImplSDL3_ProcessEvent(&event);

				if (event.type == SDL_EVENT_QUIT)
					fExitRequested = true;
				if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
					fExitRequested = true;

#if 1
				if (event.type == SDL_EVENT_KEY_DOWN)
				{
					if(event.key.keysym.sym == SDLK_ESCAPE)
						fExitRequested = true;
				}
#endif
			}

			// Start the Dear ImGui frame
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			DisplayMainWindow();			

			// Rendering
			ImGui::Render();
			
			SDL_SetRenderDrawColor(renderer, (Uint8)(g_clClearColor.x * 255), (Uint8)(g_clClearColor.y * 255), (Uint8)(g_clClearColor.z * 255), (Uint8)(g_clClearColor.w * 255));

			SDL_RenderClear(renderer);

			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());

			SDL_RenderPresent(renderer);
		}

		// Cleanup
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}	
	catch (std::exception &ex)
	{
		dcclite::LogGetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
