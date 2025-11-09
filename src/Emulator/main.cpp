// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <stdio.h>

#include <dcclite/Clock.h>
#include <dcclite/Console.h>
#include <dcclite/dcclite.h>
#include <dcclite/Log.h>
#include <dcclite/NetMessenger.h>
#include <dcclite/PathUtils.h>
#include <dcclite/Socket.h>

#include "ArduinoLib.h"


using namespace dcclite;

class TerminalClient
{
	public:
		TerminalClient(Socket &&socket);
		TerminalClient(const TerminalClient &client) = delete;
		TerminalClient(TerminalClient &&other) noexcept;

		TerminalClient &operator=(TerminalClient &&other) noexcept
		{
			if (this != &other)
			{
				m_clMessenger = std::move(other.m_clMessenger);
			}

			return *this;
		}

		bool Update();

	private:
		NetMessenger m_clMessenger;
};

TerminalClient::TerminalClient(Socket &&socket) :
	m_clMessenger(std::move(socket))
{
	//emtpy
}

TerminalClient::TerminalClient(TerminalClient &&other) noexcept :
	m_clMessenger(std::move(other.m_clMessenger))
{
	//empty
}

bool TerminalClient::Update()
{
	auto[status, msg] = m_clMessenger.Poll();

	if (status == Socket::Status::DISCONNECTED)
		return false;

	if (status == Socket::Status::OK)
	{
		dcclite::Log::Info("Received {}", msg);		

		std::stringstream stream;
		stream << msg;

		//json data;

		//stream >> data;
	}

	return true;
}

class TerminalService
{
	private:
		Socket m_clSocket;

	public:
		TerminalService();
};

TerminalService::TerminalService()
{
	if (!m_clSocket.Open(5256, Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}
}

static bool g_fExitRequested = false;

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	g_fExitRequested = true;

	return true;
}

int main(int argc, char **argv)
{
	dcclite::Init("Emulator", "Emulator.log");

	dcclite::ConsoleTryMakeNice();

	dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

	const char *deviceName = nullptr;
	for (int i = 1; i < argc; ++i)
	{
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
		{
HELP:
			printf("Usage: emulator [-h] [--help] [-d <deviceName>]\n");

			return 0;
		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			if (i + 1 == argc)
			{
				goto HELP;
			}

			deviceName = argv[++i];
		}
	}
	
	if (!ArduinoLib::Setup("LiteDecoderLib.dll", dcclite::Log::GetDefault(), deviceName) && deviceName)
	{
		//If setup returned false, rom failed to load, also if we have a device name, we need to configure it....
		dcclite::Log::Info("[main] Initializing module to create rom");

		std::stringstream stream;
		stream << "/cfg " << deviceName << ';';
		ArduinoLib::SetSerialInput(stream.str().c_str());

		ArduinoLib::Tick();

		dcclite::Log::Info("[main] Killing module");
		ArduinoLib::Finalize();

		dcclite::Log::Info("[main] Reloading module");
		//try again...
		if (!ArduinoLib::Setup("LiteDecoderLib.dll", dcclite::Log::GetDefault(), deviceName))
		{
			dcclite::Log::Critical("[main] Failed to reload arduino lib to use new rom");

			return EXIT_FAILURE;
		}		
	}

	dcclite::Log::Info("[main] Setup complete, starting main loop");

	//TerminalService terminalService;

	//ArduinoLib::SetSerialInput("/cfg NoName;sv;");	

	dcclite::Clock clock;

	while(!g_fExitRequested)
	{
		if (!clock.Tick(std::chrono::milliseconds{ 10 }))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
			continue;
		}

		ArduinoLib::Tick();
	}

	ArduinoLib::Finalize();

#if 0	
	ArduinoLib::Tick();

	//CE:83:5A:D3:E5:E5
	ArduinoLib::SetSerialInput("/cfg emul CE.83.5A.D3.E5.E5 7202 127.0.0.1 8989;");

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("/sv;");

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C TRGR 3 5 1 C>");		

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(6, LOW);	

	ArduinoLib::Tick();	

	ArduinoLib::SetPinDigitalVoltage(5, LOW);

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(6, HIGH);

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(5, HIGH);

	ArduinoLib::Tick();
#endif

	return 0;
}

