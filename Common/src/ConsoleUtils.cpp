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

	void InstallConsoleEventHandler(ConsoleEventCallback_t callback)
	{
		g_Callback = callback;

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, FALSE);
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE);
	}
}