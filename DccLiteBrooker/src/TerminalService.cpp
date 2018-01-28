#include "TerminalService.h"

#include <stdexcept>

#include <boost/log/trivial.hpp>

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name, params); }
);

class TerminalClient
{

};


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

	if (status == dcclite::Socket::Status::OK)
	{
		BOOST_LOG_TRIVIAL(info) << "[TermnialService] Client connected";
	}
}


