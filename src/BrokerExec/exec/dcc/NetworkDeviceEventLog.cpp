// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NetworkDeviceEventLog.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <magic_enum/magic_enum.hpp>

#include <dcclite/Object.h>

namespace dcclite::broker::exec::dcc
{
	size_t NetworkDeviceEventLog::PushEvent(EventType type, std::string_view info)
	{
		m_vecEvents.push_back(
			Event
			{
				.m_kType = type,
				.m_tTime = std::chrono::system_clock::now(),
				.m_svInfo = info
			}
		);

		return m_vecEvents.size() - 1;
	}

	static void SerializeSingleEvent(dcclite::JsonOutputStream_t &obj, const NetworkDeviceEventLog::Event &event)
	{
		auto timeStr = fmt::format("{:%Y/%m/%d %H:%M:%S}", event.m_tTime);

		obj.AddStringValue("type", magic_enum::enum_name(event.m_kType));
		obj.AddStringValue("time", timeStr);
		obj.AddStringValue("info", event.m_svInfo);
	}

	void NetworkDeviceEventLog::SerializeEvent(dcclite::JsonOutputStream_t &stream, const size_t pos) const
	{
		auto logArray = stream.AddArray("eventsLog");
		auto eventObj = logArray.AddObject();

		SerializeSingleEvent(eventObj, m_vecEvents.at(pos));
	}

	void NetworkDeviceEventLog::Serialize(dcclite::JsonOutputStream_t &stream) const
	{	
		if(m_vecEvents.empty())
			return;		

		auto logArray = stream.AddArray("eventsLog");

		for (const auto &event : m_vecEvents)
		{
			auto eventObject = logArray.AddObject();

			SerializeSingleEvent(eventObject, event);
		}		
	}	
}
