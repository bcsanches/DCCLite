#include <stdio.h>

#include <plog/Log.h>

#include "Arduino.h"

#include "ConsoleUtils.h"
#include "LogUtils.h"
#include "NetMessenger.h"
#include "Socket.h"

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
		LOG_INFO << "Received " << msg;

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
	dcclite::InitLog("Emulator%N.log");

	dcclite::ConsoleMakeNice();

	dcclite::ConsoleInstallEventHandler(ConsoleCtrlHandler);

#ifdef _DEBUG
	ArduinoLib::Setup("LiteDecoderLib_d.dll");
#else
	ArduinoLib::Setup("LiteDecoderLib.dll");
#endif

	TerminalService terminalService;

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C OUTD 1 7 0>");

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C TRGR 2 6 1 T>");

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

	return 0;
}

