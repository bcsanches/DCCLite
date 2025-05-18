// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <vector>

#include <thread>

#include <dcclite/Socket.h>

#include "../sys/Service.h"
#include "../sys/EventHub.h"

namespace dcclite::broker
{
	class CmdHostService;
	class TerminalClient;

	class ITerminalServiceClientProxy
	{
		public:
			virtual void Async_DisconnectClient(TerminalClient &client) = 0;
	};

	class TerminalService : public Service, EventHub::IEventTarget, ITerminalServiceClientProxy
	{
		private:		
			dcclite::Socket m_clSocket;

			std::vector <std::unique_ptr<TerminalClient>> m_vecClients;
			
			std::thread m_thListenThread;	

			CmdHostService &m_rclCmdHost;

		public:
			static const char *TYPE_NAME;

			static void RegisterFactory();

			TerminalService(RName name, Broker &broker, const rapidjson::Value &params, CmdHostService &cmdHost);

			virtual ~TerminalService();			

			const char *GetTypeName() const noexcept override { return TYPE_NAME; }

			typedef CmdHostService Requirement_t;

		private:
			void ListenThreadProc(const int port);

			void OnAcceptConnection(const dcclite::NetworkAddress &address, dcclite::Socket &&s);

			void OnClientDisconnect(TerminalClient &client);

			void Async_DisconnectClient(TerminalClient &client) override;

			friend class TerminalServiceClientDisconnectedEvent;
			friend class TerminalServiceAcceptConnectionEvent;
	};
}