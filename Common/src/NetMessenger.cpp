#include "NetMessenger.h"

namespace dcclite
{
	NetMessenger::NetMessenger(Socket socket) :
		m_clSocket(std::move(socket))
	{
		//empty
	}

	std::tuple<Socket::Status, std::string> NetMessenger::Poll()
	{
		char tmpBuffer[1024];

		auto[status, size] = m_clSocket.Receive(tmpBuffer, sizeof(tmpBuffer));

		if ((status == Socket::Status::DISCONNECTED) && (m_lstMessages.empty()))
			return std::make_tuple(Socket::Status::DISCONNECTED, std::string{});

		if (status == Socket::Status::OK)
		{
			m_strIncomingMessage.append(tmpBuffer, size);
			
			for (auto pos = m_strIncomingMessage.find("\r\n"); pos != std::string::npos; pos = m_strIncomingMessage.find("\r\n"))
			{								
				m_lstMessages.emplace_back(m_strIncomingMessage.substr(0, pos));

				m_strIncomingMessage.erase(0, m_strIncomingMessage[pos]);
			}
		}

		return this->PollInternalQueue();		
	}

	std::tuple<Socket::Status, std::string> NetMessenger::PollInternalQueue()
	{
		if (m_lstMessages.empty())
			return std::make_tuple(Socket::Status::EMPTY, std::string{});
		
		std::string output{ std::move(m_lstMessages.front()) };

		m_lstMessages.pop_front();

		return std::make_tuple(Socket::Status::OK, std::move(output));		
	}
}
