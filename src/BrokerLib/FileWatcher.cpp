// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FileWatcher.h"

#include "Thinker.h"

#include <fmt/format.h>
#include <Log.h>
#include <lfwatch.h>
#include <map>

using namespace std::chrono_literals;

namespace FileWatcher
{		
	static void PumpEvents(const dcclite::Clock::TimePoint_t tp);

	class DirectoryWatcher;

	static lfw::Watcher gWatcher;
	static dcclite::broker::Thinker g_Thinker{ "FileWatcher::PumpEvents", PumpEvents};
	static std::map<dcclite::fs::path, DirectoryWatcher> g_mapWatchers;

	constexpr auto DEFAULT_INTERVAL = 1s;

	inline uint32_t FlagBroker2Lfw(const uint32_t flags)
	{
		uint32_t newFlag = 0;

		if (flags & FW_MODIFIED)
			newFlag |= lfw::FILE_MODIFIED;

		return newFlag;
	}

	inline uint32_t FlagLfw2Broker(const uint32_t flags)
	{
		uint32_t newFlag = 0;

		if(flags == lfw::FILE_MODIFIED)
			newFlag |= FW_MODIFIED;

		return newFlag;
	}

	class DirectoryWatcher
	{
		public:			
			void AddHandle(std::string fileName, uint32_t flags, const Callback_t &callback)
			{				
				if(m_mapHandles.find(fileName) != m_mapHandles.end())
					throw std::logic_error(fmt::format("[FileWatcher::DirectoryWatcher::AddHandler] Duplicated handle for {}", fileName));

				Handle h;

				h.m_fFlags = flags;
				h.m_pfnCallback = callback;

				m_mapHandles.insert(std::make_pair(std::move(fileName), h));				
			}

			void RemoveHandle(const std::string &fileName)
			{
				m_mapHandles.erase(fileName);
			}

			bool IsEmpty() const
			{
				return m_mapHandles.empty();
			}

			void OnFileEvent(const lfw::EventData &data)
			{
				auto it = m_mapHandles.find(data.fname);
				if(it == m_mapHandles.end())
					return;

				auto flags = FlagLfw2Broker(data.event);

				auto &handle = it->second;

				if (handle.m_fFlags & flags)
				{
					Event ev;

					ev.m_fFlags = handle.m_fFlags;
					ev.m_strFileName = data.fname;
					ev.m_strPath = data.dir;

					it->second.m_pfnCallback(ev);
				}
			}

		private:	
			struct Handle
			{
				uint32_t m_fFlags;
				Callback_t m_pfnCallback;
			};

			std::map<std::string, Handle> m_mapHandles;
	};	

	static void PumpEvents(const dcclite::Clock::TimePoint_t tp)
	{
		gWatcher.update();

		g_Thinker.SetNext(tp + DEFAULT_INTERVAL);
	}	

	void WatchFile(const dcclite::fs::path &fileName, const uint32_t flags, const Callback_t &callback)
	{
		assert(flags);

		auto filePath = fileName.parent_path();

		auto it = g_mapWatchers.find(filePath);
		if (it == g_mapWatchers.end())
		{
			it = g_mapWatchers.insert(std::make_pair(filePath, DirectoryWatcher{})).first;

			auto watcher = &it->second;
			
			gWatcher.watch(filePath.string(), FlagBroker2Lfw(flags), [watcher](const lfw::EventData &data){
				watcher->OnFileEvent(data);
			});
		}

		it->second.AddHandle(fileName.filename().string(), flags, callback);

		if (!g_Thinker.IsScheduled())
		{
			g_Thinker.SetNext(dcclite::Clock::DefaultClock_t::time_point{});
		}
	}

	void UnwatchFile(const dcclite::fs::path &fileName)
	{
		const auto filePath = fileName.parent_path();
		const auto name = fileName.filename().string();

		auto it = g_mapWatchers.find(filePath);
		if (it == g_mapWatchers.end())
		{
			dcclite::Log::Warn("[FileWatcher::UnwatchFile] Path {} not registered for file {}", filePath.string(), name);

			return;
		}

		it->second.RemoveHandle(name);

		if (it->second.IsEmpty())
		{
			g_mapWatchers.erase(it);

			gWatcher.remove(filePath.string());

			if (g_mapWatchers.empty())
				g_Thinker.Cancel();
		}		
	}	
}


