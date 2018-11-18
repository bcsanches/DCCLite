#include <stdio.h>

#include "ConsoleUtils.h"

#include "Clock.h"
#include "NetMessenger.h"
#include "Socket.h"

#include "ArduinoLib.h"

#include <spdlog/logger.h>

#include "LogUtils.h"

using namespace dcclite;

class TerminalClient
{
	public:
		TerminalClient(Socket &&socket);
		TerminalClient(const TerminalClient &client) = delete;
		TerminalClient(TerminalClient &&other);

		TerminalClient &operator=(TerminalClient &&other)
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

TerminalClient::TerminalClient(TerminalClient &&other) :
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

#ifdef _DEBUG
	ArduinoLib::Setup("LiteDecoderLib_d.dll", dcclite::LogGetDefault());
#else
	ArduinoLib::Setup("LiteDecoderLib.dll");
#endif

	TerminalService terminalService;		

	//ArduinoLib::SetSerialInput("/cfg emul 206.174.184.251.21.20 7202 127.0.0.1 8989;");

	//ArduinoLib::Tick();

	//ArduinoLib::SetSerialInput("/sv;");

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

