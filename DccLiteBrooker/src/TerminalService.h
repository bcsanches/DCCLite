#pragma once

#include "Service.h"

#include "Socket.h"

class ConnectionHandler;

class TerminalService : public Service
{
	private:		
		dcclite::Socket m_clSocket;

	public:
		TerminalService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params);

		virtual ~TerminalService();

		virtual void Update();			
};
