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

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

constexpr auto BUILD_NUM = DCCLITE_VERSION;

static bool fExitRequested = false;

//static TerminalCmd g_ShutdownCmd{ "shutdown" };

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

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);

		// Our state
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
				ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

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

			// Rendering
			ImGui::Render();
			
			SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));

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
