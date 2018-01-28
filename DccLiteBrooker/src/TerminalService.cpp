#include "TerminalService.h"

#include <stdexcept>

#include <boost/log/trivial.hpp>

using namespace dcclite;

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name, params); }
);

class TerminalClient
{
	public:
		TerminalClient(Socket socket);
		TerminalClient(const TerminalClient &socket) = delete;
		TerminalClient(TerminalClient &&other);

		TerminalClient &operator=(TerminalClient &&other)
		{
			if (this != &other)
			{
				m_clSocket = std::move(other.m_clSocket);
			}

			return *this;
		}

		bool Update();

	private:
		Socket m_clSocket;
};

TerminalClient::TerminalClient(Socket socket):
	m_clSocket(std::move(socket))
{
	//emtpy
}

TerminalClient::TerminalClient(TerminalClient &&other) :
	m_clSocket(std::move(other.m_clSocket))
{
	//empty
}

bool TerminalClient::Update()
{
	char buffer[256];

	auto [status, size]  = m_clSocket.Receive(buffer, sizeof(buffer));	

	if (status == Socket::Status::DISCONNECTED)
		return false;

	if (status == Socket::Status::OK)
	{
		buffer[std::min(sizeof(buffer)-1,  size)] = 0;
		BOOST_LOG_TRIVIAL(info) << "Received " << buffer;
	}

	return true;
}

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) :
	Service(serviceClass, name, params)	
{	
	if (!m_clSocket.TryOpen(params["port"].get<unsigned short>(), dcclite::Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.TryListen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}
}


TerminalService::~TerminalService()
{
	//empty
}

void TerminalService::Update()
{
	auto [status, socket, address] = m_clSocket.TryAccept();

	if (status == Socket::Status::OK)
	{
		BOOST_LOG_TRIVIAL(info) << "[TermnialService] Client connected";

		m_vecClients.emplace_back(std::move(socket));
	}

	for (size_t i = 0; i < m_vecClients.size(); ++i)
	{
		auto &client = m_vecClients[i];

		if (!client.Update())
		{
			BOOST_LOG_TRIVIAL(info) << "[TermnialService] Client disconnected";

			m_vecClients.erase(m_vecClients.begin() + i);
		}
	}
}


