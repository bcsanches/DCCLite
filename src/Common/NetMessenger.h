#pragma once

#include <deque>
#include <string>
#include <tuple>

#include "Socket.h"

namespace dcclite
{
	class NetMessenger
	{
		public:
			NetMessenger(Socket &&socket);


			std::tuple<Socket::Status, std::string> Poll();

		private:
			std::tuple<Socket::Status, std::string> PollInternalQueue();

		private:
			Socket m_clSocket;

			std::deque<std::string> m_lstMessages;

			std::string m_strIncomingMessage;
	};
}
