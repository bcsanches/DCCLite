// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NetMessenger.h"

#include <algorithm>
#include <string.h>

namespace dcclite
{
	NetMessenger::NetMessenger(Socket &&socket, const char *separator, const char *initialBuffer) :			
		m_clSocket(std::move(socket))
	{
		if (!separator)
			throw new std::logic_error("[NetMessenger] separator cannot be null");
		
		m_pszSeparator = separator;
		m_uSeparatorLength = strlen(separator);

		if (initialBuffer)
		{
			m_strIncomingMessage.append(initialBuffer);

			this->ParseIncomingMessage();
		}
	}	

	NetMessenger::NetMessenger(NetMessenger &&rhs) noexcept:
		m_clSocket{ std::move(rhs.m_clSocket) },
		m_pszSeparator{rhs.m_pszSeparator},
		m_uSeparatorLength{rhs.m_uSeparatorLength},
		m_lstMessages{ std::move(rhs.m_lstMessages) },
		m_strIncomingMessage{ std::move(rhs.m_strIncomingMessage) }	
	{
		//empty
	}

	void NetMessenger::ParseIncomingMessage()
	{
		for (auto pos = m_strIncomingMessage.find(m_pszSeparator); pos != std::string::npos; pos = m_strIncomingMessage.find(m_pszSeparator))
		{
			if(pos)
				m_lstMessages.emplace_back(m_strIncomingMessage.substr(0, pos));

			m_strIncomingMessage.erase(0, pos + m_uSeparatorLength);
		}
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
					
			this->ParseIncomingMessage();
		}

		return this->PollInternalQueue();		
	}

	bool NetMessenger::Send(const NetworkAddress &destination, std::string_view msg)
	{
		if (!m_clSocket.Send(destination, msg.data(), msg.length()))
			return false;
		
		return m_clSocket.Send(destination, "\r\n", 2);
	}

	bool NetMessenger::Send(std::string_view msg)
	{
		{
			auto [status, size] = m_clSocket.Send(msg.data(), msg.length());
			if (status == Socket::Status::DISCONNECTED)
				return false;
		}

		auto [status, size] = m_clSocket.Send("\r\n", 2);
		return status != Socket::Status::DISCONNECTED;
	}

	std::tuple<Socket::Status, std::string> NetMessenger::PollInternalQueue()
	{
		if (m_lstMessages.empty())
			return std::make_tuple(Socket::Status::WOULD_BLOCK, std::string{});
		
		std::string output{ std::move(m_lstMessages.front()) };

		m_lstMessages.pop_front();

		return std::make_tuple(Socket::Status::OK, std::move(output));		
	}
}
