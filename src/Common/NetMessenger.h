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
			NetMessenger(Socket &&socket, const char *separator = "\r\n", const char *initialBuffer = "");
			NetMessenger(NetMessenger&& rhs) noexcept;

			NetMessenger &operator=(NetMessenger&& rhs) noexcept = default;

			NetMessenger(const NetMessenger &rhs) = delete;			
			const NetMessenger operator=(const NetMessenger& rhs) = delete;			

			std::tuple<Socket::Status, std::string> Poll();
			std::tuple<Socket::Status, std::string> SyncPoll();

			bool Send(const NetworkAddress &destination, std::string_view msg);
			bool Send(std::string_view msg);

			void Close();			

		private:
			std::tuple<Socket::Status, std::string> PollInternalQueue();

			void ParseIncomingMessage();

			Socket::Status WaitData();

		private:
			Socket m_clSocket;
			
			const char *m_pszSeparator;
			size_t		m_uSeparatorLength;

			std::deque<std::string> m_lstMessages;

			std::string m_strIncomingMessage;
	};
}
