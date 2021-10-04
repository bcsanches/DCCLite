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

#include <deque>
#include <string>
#include <tuple>
#include <vector>

#include "Socket.h"

namespace dcclite
{
	class NetMessenger
	{
		public:
			NetMessenger(Socket &&socket, const char *separator = "\r\n");
			NetMessenger(Socket &&socket, std::initializer_list<const char *> separators);
			NetMessenger(NetMessenger&& rhs) = default;

			NetMessenger &operator=(NetMessenger&& rhs) noexcept = default;

			NetMessenger(const NetMessenger &rhs) = delete;			
			const NetMessenger operator=(const NetMessenger& rhs) = delete;			

			std::tuple<Socket::Status, std::string> Poll();

			bool Send(const NetworkAddress &destination, std::string_view msg);

		private:
			std::tuple<Socket::Status, std::string> PollInternalQueue();

		private:
			Socket m_clSocket;

			struct Separator
			{
				Separator(const char *string, size_t length) :
					m_szString(string),
					m_szLength(length)
				{
					//empty
				}

				const char	*m_szString;
				size_t		m_szLength;
			};

			std::vector<Separator> m_vecSeparators;			

			std::deque<std::string> m_lstMessages;

			std::string m_strIncomingMessage;
	};
}
