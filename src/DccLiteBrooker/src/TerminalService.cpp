#include "TerminalService.h"

#include <sstream>
#include <stdexcept>

#include <plog/Log.h>

#include "json.hpp"
#include "NetMessenger.h"

using json = nlohmann::json;
using namespace dcclite;

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name, params); }
);

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

TerminalClient::TerminalClient(Socket &&socket):
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

		json data;

		stream >> data;


	}

	return true;
}

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) :
	Service(serviceClass, name, params)	
{	
	if (!m_clSocket.Open(params["port"].get<unsigned short>(), dcclite::Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
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
		LOG_INFO << "[TermnialService] Client connected " << address.GetIpString();

		m_vecClients.emplace_back(std::move(socket));
	}

	for (size_t i = 0; i < m_vecClients.size(); ++i)
	{
		auto &client = m_vecClients[i];

		if (!client.Update())
		{
			LOG_INFO << "[TermnialService] Client disconnected";

			m_vecClients.erase(m_vecClients.begin() + i);
		}
	}
}


