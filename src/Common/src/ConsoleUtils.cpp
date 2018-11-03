#include "ConsoleUtils.h"

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

	void ConsoleMakeNice()
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_FONT_INFOEX fontInfo;
		fontInfo.cbSize = sizeof(fontInfo);

		if (!GetCurrentConsoleFontEx(hStdout, false, &fontInfo))
			return;

		fontInfo.dwFontSize.X = 7;
		fontInfo.dwFontSize.Y = 14;

		SetCurrentConsoleFontEx(hStdout, false, &fontInfo);

		CONSOLE_SCREEN_BUFFER_INFOEX bufferInfo = { 0 };
		bufferInfo.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

		if (!GetConsoleScreenBufferInfoEx(hStdout, &bufferInfo))
			return;				

		bufferInfo.dwSize.X = 120;
		bufferInfo.dwSize.Y = 3000;

		bufferInfo.srWindow.Right = 119;
		bufferInfo.srWindow.Bottom = 49;

		bufferInfo.ColorTable[0] = RGB(1, 36, 86);
		bufferInfo.ColorTable[7] = RGB(238, 237, 240);

		SetConsoleScreenBufferInfoEx(hStdout, &bufferInfo);			
	}
}
