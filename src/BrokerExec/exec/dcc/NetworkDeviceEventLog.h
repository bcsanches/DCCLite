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

#include <chrono>
#include <string_view>
#include <vector>

#include <dcclite/Object.h>

namespace dcclite::broker::exec::dcc
{
	class NetworkDeviceEventLog
	{
		public:
			enum class EventType
			{
				CONNECTED,
				MISSED_PONG,
				GOT_PONG,
				DISCONNECTED
			};

			struct Event
			{
				EventType								m_kType;

				std::chrono::system_clock::time_point	m_tTime;
				std::string_view						m_svInfo;
			};

			size_t PushEvent(EventType type, std::string_view info);

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeEvent(dcclite::JsonOutputStream_t &stream, const size_t pos) const;			

		private:
			std::vector<Event>	m_vecEvents;
	};
}
