// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "TerminalService.h"

#include <dcclite/Util.h>

#include <sys/BonjourService.h>
#include <sys/ServiceFactory.h>
#include <sys/ZeroConfSystem.h>

#include "CmdHostService.h"
#include "TerminalClient.h"
#include "TerminalServiceCmds.h"

namespace dcclite::broker::shell::terminal
{
	const char *TerminalService::TYPE_NAME = "TerminalService";	

	using namespace dcclite;	

	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalService Events
	//
	/////////////////////////////////////////////////////////////////////////////
	class TerminalServiceAcceptConnectionEvent: public sys::EventHub::IEvent
	{
		public:
			TerminalServiceAcceptConnectionEvent(TerminalService &target, const dcclite::NetworkAddress &address, Socket s):
				IEvent(target),
				m_clSocket{ std::move(s) },
				m_clAddress{ address }

			{
				//empty
			}

			void Fire() override
			{
				static_cast<TerminalService &>(this->GetTarget()).OnAcceptConnection(m_clAddress, std::move(m_clSocket));
			}

		private:
			Socket m_clSocket;
			dcclite::NetworkAddress m_clAddress;
	};

	class TerminalServiceClientDisconnectedEvent: public sys::EventHub::IEvent
	{
		public:
			TerminalServiceClientDisconnectedEvent(TerminalService &target, TerminalClient &client):
				IEvent(target),
				m_rclClient(client)
			{
				//empty
			}

			void Fire() override
			{
				static_cast<TerminalService &>(this->GetTarget()).OnClientDisconnect(m_rclClient);
			}

		private:
			TerminalClient &m_rclClient;
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalService
	//
	/////////////////////////////////////////////////////////////////////////////
	void TerminalService::RegisterFactory()
	{
		static sys::GenericServiceWithDependenciesFactory<TerminalService> g_clTerminalServiceFactory;
	}

	TerminalService::TerminalService(RName name, sys::Broker &broker, const rapidjson::Value &params, CmdHostService &cmdHost) :
		Service(name, broker, params),
		m_rclCmdHost{cmdHost}
	{
		const auto port = dcclite::json::TryGetDefaultInt(params, "port", DEFAULT_TERMINAL_SERVER_PORT);
		
		m_thListenThread = std::thread{ [port, this] {this->ListenThreadProc(port); } };		
		dcclite::SetThreadName(m_thListenThread, "TerminalService::ListenThread");

		if(auto bonjourService = m_rclBroker.TryFindServiceByType<sys::BonjourService>())		
			bonjourService->Register("terminal", "dcclite", sys::NetworkProtocol::TCP, port, 36);

		sys::ZeroConfSystem::Register(this->GetTypeName(), port);

		RegisterBaseTerminalCmds(cmdHost);
	}

	TerminalService::~TerminalService()
	{
		//close socket, so listen thread stops...
		m_clSocket.Close();

		//kill all clients...
		m_vecClients.clear();

		//wait for listen thread to finish, so we are sure no more events will be posted
		m_thListenThread.join();

		//Cancel any events, because no one will be able to handle those
		sys::EventHub::CancelEvents(*this);
	}

	void TerminalService::OnClientDisconnect(TerminalClient &client)
	{
		auto it = std::find_if(m_vecClients.begin(), m_vecClients.end(), [&client](auto &item)
			{
				return item.get() == &client;
			}
		);

		assert(it != m_vecClients.end());

		m_vecClients.erase(it);

		dcclite::Log::Info("[TerminalService] Client disconnected");
	}

	void TerminalService::Async_DisconnectClient(TerminalClient &client)
	{
		sys::EventHub::PostEvent< TerminalServiceClientDisconnectedEvent>(std::ref(*this), std::ref(client));
	}

	void TerminalService::OnAcceptConnection(const dcclite::NetworkAddress &address, dcclite::Socket &&s)
	{
		dcclite::Log::Info("[TerminalService] Client connected {}", address.GetIpString());

		ITerminalServiceClientProxy &proxy = *this;

		m_vecClients.push_back(
			std::make_unique<TerminalClient>(
				proxy,
				m_rclCmdHost,
				m_rclBroker, 
				*this,
				address, 
				std::move(s)
			)
		);
	}

	void TerminalService::ListenThreadProc(const int port)
	{	
		if (!m_clSocket.Open(port, dcclite::Socket::Type::STREAM, Socket::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error("[TerminalService] Cannot open socket");
		}

		if (!m_clSocket.Listen())
		{
			throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
		}

		dcclite::Log::Info("[TerminalService] Started, listening on port {}", port);

		for (;;)
		{
			auto [status, socket, address] = m_clSocket.TryAccept();

			if (status != Socket::Status::OK)
				break;			
			
			sys::EventHub::PostEvent<TerminalServiceAcceptConnectionEvent>(
				std::ref(*this), 
				address, 
				std::move(socket)				
			);
		}
	}	
}
