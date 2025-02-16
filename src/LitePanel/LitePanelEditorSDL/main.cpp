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

#include <dcclite/Clock.h>
#include <dcclite/Console.h>
#include <dcclite/dcclite.h>
#include <dcclite/Log.h>

#include <spdlog/logger.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"

#include "PanelEditorApp.h"

constexpr auto BUILD_NUM = DCCLITE_VERSION;

static bool g_fExitRequested = false;

static ImVec4 g_clClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


int main(int argc, char **argv)
{				
	try
	{ 
		dcclite::Init("LitePanelSDL", "LitePanelSDL.log");		

#ifndef DEBUG
		dcclite::Log::GetDefault()->set_level(spdlog::level::trace);
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

			SDL_Quit();
			return -1;
		}

		SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
		if (renderer == nullptr)
		{
			dcclite::Log::Critical("[LitePanelSDL] Error: SDL_CreateRenderer(): %s\n", SDL_GetError());		

			SDL_DestroyWindow(window);
			SDL_Quit(); 

			return -1;
		}

		SDL_SetRenderVSync(renderer, 1);

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
		
		//io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 6.0f);
		//io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontDefault();
	

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);		

		dcclite::PanelEditor::PanelEditorApp app;

		using namespace std::chrono_literals;
		unsigned frameCount = 0;
		auto startTime = dcclite::Clock::DefaultClock_t::now();

		while (!g_fExitRequested)
		{
#if 1
			auto now = dcclite::Clock::DefaultClock_t::now();
			auto delta = now - startTime;
			if (delta < 20ms)
			{
				std::this_thread::sleep_for(delta - 20ms);
				continue;
			}

			++frameCount;
			if (delta >= std::chrono::seconds{ 1 })
			{
				startTime += std::chrono::seconds{ 1 };

				//dcclite::Log::Debug("[{}]", frameCount);
				frameCount = 0;
			}
#endif

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
					g_fExitRequested = true;
				if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
					g_fExitRequested = true;

				if (event.type == SDL_EVENT_KEY_DOWN)
					app.HandleEvent(event.key);
			}

			// Start the Dear ImGui frame
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			if (!app.Display())
				g_fExitRequested = true;

			// Rendering
			ImGui::Render();
			
			SDL_SetRenderDrawColor(renderer, (Uint8)(g_clClearColor.x * 255), (Uint8)(g_clClearColor.y * 255), (Uint8)(g_clClearColor.z * 255), (Uint8)(g_clClearColor.w * 255));

			SDL_RenderClear(renderer);

			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

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
		dcclite::Log::GetDefault()->critical("caught {}", ex.what());
	}

	return 0;
}
