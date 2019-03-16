#pragma once

#include <vector>

#include "Service.h"

#include "Socket.h"

class TerminalClient;

class TerminalService : public Service
{
	private:		
		dcclite::Socket m_clSocket;

		std::vector<TerminalClient> m_vecClients;

	public:
		TerminalService(const ServiceClass &serviceClass, const std::string &name, const rapidjson::Value &params, const Project &project);

		virtual ~TerminalService();

		virtual void Update(const dcclite::Clock &clock) override;

		virtual const char *GetTypeName() const noexcept { return "TerminalService"; }
};
