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
				if (event.type == SDL_EVENT_QUIT)
					fExitRequested = true;
				if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
					fExitRequested = true;

				if (event.type == SDL_EVENT_KEY_DOWN)
				{
					if(event.key.keysym.sym == SDLK_ESCAPE)
						fExitRequested = true;
				}
			}

			//SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));

			SDL_RenderClear(renderer);			
			SDL_RenderPresent(renderer);
		}

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
