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

#include "Clock.h"
#include "ConsoleUtils.h"
#include "LogUtils.h"
#include "NetMessenger.h"
#include "PathUtils.h"
#include "Socket.h"

#include "ArduinoLib.h"

#include <spdlog/logger.h>


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
		dcclite::LogGetDefault()->info("Received {}", msg);		

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
	if (!m_clSocket.Open(7202, Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}
}

static bool fExitRequested = false;

static bool ConsoleCtrlHandler(dcclite::ConsoleEvent event)
{
	fExitRequested = true;

	return true;
}

int main(int, char **)
{
	dcclite::LogInit("Emulator.log");

	dcclite::ConsoleTryMakeNice();

	dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

	dcclite::PathUtils::InitAppFolders("Emulator");

	ArduinoLib::Setup("LiteDecoderLib.dll", dcclite::LogGetDefault());

	TerminalService terminalService;		

#if 1
	//format: cfg <nodeName> <mac> <port> <srvipv4>	<srvport>	
	//ArduinoLib::SetSerialInput("/cfg RelayStagingA 206.174.184.251.21.20 7202 192.168.0.20 8989;");
	//ArduinoLib::SetSerialInput("/cfg Emulator 206.174.184.251.21.20 7202 192.168.0.20 8989;");
	//ArduinoLib::SetSerialInput("/cfg TestDevice 242.69.116.44.41.93 7202 8989 a;");

	ArduinoLib::Tick();

	//ArduinoLib::SetSerialInput("/sv;");
#endif

	dcclite::Clock clock;

	while(!fExitRequested)
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

