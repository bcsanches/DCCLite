// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ConsoleUtils.h"

#include "LogUtils.h"

#include <spdlog/spdlog.h>

#include <Windows.h>

namespace dcclite
{
	static ConsoleEventCallback_t g_Callback = nullptr;

	static BOOL ConsoleCtrlHandler(DWORD dwCtrlType)
	{
		switch (dwCtrlType)
		{
			case CTRL_C_EVENT:
				return g_Callback(ConsoleEvent::CTRL_C);

			case CTRL_BREAK_EVENT:
				return g_Callback(ConsoleEvent::CTRL_BREAK);

			case CTRL_CLOSE_EVENT:
				return g_Callback(ConsoleEvent::CLOSE);

			case CTRL_LOGOFF_EVENT:
				return g_Callback(ConsoleEvent::LOGOFF);

			case CTRL_SHUTDOWN_EVENT:
				return g_Callback(ConsoleEvent::SHUTDOWN);

			default:
				return false;
		}
	}

	void ConsoleInstallEventHandler(ConsoleEventCallback_t callback)
	{
		g_Callback = callback;

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, FALSE);
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE);
	}

	bool ConsoleTryMakeNice()
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_FONT_INFOEX fontInfo;
		fontInfo.cbSize = sizeof(fontInfo);

		if (!GetCurrentConsoleFontEx(hStdout, false, &fontInfo))
		{
			spdlog::error("GetCurrentConsoleFontEx failed");
			return false;
		}

		fontInfo.dwFontSize.X = 7;
		fontInfo.dwFontSize.Y = 14;

		SetCurrentConsoleFontEx(hStdout, false, &fontInfo);

		CONSOLE_SCREEN_BUFFER_INFOEX bufferInfo = { 0 };
		bufferInfo.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

		if (!GetConsoleScreenBufferInfoEx(hStdout, &bufferInfo))
		{
			spdlog::error("GetConsoleScreenBufferInfoEx failed");
			return false;
		}

		bufferInfo.dwSize.X = 120;
		bufferInfo.dwSize.Y = 3000;

		bufferInfo.srWindow.Right = 119;
		bufferInfo.srWindow.Bottom = 49;

		bufferInfo.ColorTable[0] = RGB(1, 36, 86);
		bufferInfo.ColorTable[7] = RGB(238, 237, 240);

		SetConsoleScreenBufferInfoEx(hStdout, &bufferInfo);	

#if 1
		if (!SetConsoleOutputCP(CP_UTF8))
		{
			spdlog::error("SetConsoleOutputCP failed");
			return false;
		}
#endif

		return true;
	}
}
