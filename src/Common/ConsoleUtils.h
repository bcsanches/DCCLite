#pragma once

namespace dcclite
{
	enum class ConsoleEvent
	{
		CTRL_C,
		CTRL_BREAK,
		CLOSE,
		LOGOFF,
		SHUTDOWN
	};

	typedef bool (*ConsoleEventCallback_t)(ConsoleEvent );

	extern void ConsoleInstallEventHandler(ConsoleEventCallback_t callback);

	extern bool ConsoleTryMakeNice();


}

