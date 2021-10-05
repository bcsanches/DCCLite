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
	NetMessenger::NetMessenger(Socket &&socket, const char *separator) :			
		m_clSocket(std::move(socket))
	{
		if (!separator)
			throw new std::logic_error("[NetMessenger] separator cannot be null");
		
		m_vecSeparators.push_back(Separator{ separator, strlen(separator) });		
	}

	NetMessenger::NetMessenger(Socket &&socket, std::initializer_list<const char *> separators) :
		m_clSocket(std::move(socket))		
	{
		if (separators.size() == 0)
			throw new std::logic_error("[NetMessenger] separator cannot be empty");

		for(auto sep : separators)		
			m_vecSeparators.push_back(Separator{ sep, strlen(sep) });

		std::sort(m_vecSeparators.begin(), m_vecSeparators.end(), [](const Separator &lhs, const Separator &rhs)
			{
				return rhs.m_szLength < lhs.m_szLength;					
			}
		);
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
		
NEXT_MSG:
			for (auto &separator : m_vecSeparators)
			{
				for (auto pos = m_strIncomingMessage.find(separator.m_szString); pos != std::string::npos; pos = m_strIncomingMessage.find(separator.m_szString))
				{
					m_lstMessages.emplace_back(m_strIncomingMessage.substr(0, pos));

					m_strIncomingMessage.erase(0, pos + separator.m_szLength);

					goto NEXT_MSG;
				}
			}			
		}

		return this->PollInternalQueue();		
	}

	bool NetMessenger::Send(const NetworkAddress &destination, std::string_view msg)
	{
		if (!m_clSocket.Send(destination, msg.data(), msg.length()))
			return false;
		
		return m_clSocket.Send(destination, "\r\n", 2);
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
