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

#include <map>

#include <fmt/format.h>

#include <ldmonitor/DirectoryMonitor.h>

#include <dcclite/Log.h>

#include "EventHub.h"
#include "Timeouts.h"
#include "Thinker.h"

namespace FileWatcher
{			
	class EventTarget : public dcclite::broker::EventHub::IEventTarget
	{
		public:
			EventTarget()
			{
				//empty
			}
	};

	class DirectoryWatcher;
		
	static std::mutex										g_lckMutex;
	static std::map<dcclite::fs::path, DirectoryWatcher>	g_mapWatchers;
	static EventTarget										g_clSentinel;

	static bool IsHandleValid(const ldmonitor::fs::path &path, const std::string &fileName) noexcept;	

	class DirectoryWatcher
	{
		public:			
			bool TryAddHandle(std::string fileName, const Callback_t &callback)
			{			
				if(IsHandleValid(fileName))				
				{
					dcclite::Log::Warn("[FileWatcher::DirectoryWatcher::AddHandler] Duplicated handle for {}", fileName);

					return false;
				}
					
				Handle h;
				
				h.m_pfnCallback = callback;

				m_mapHandles.insert(std::make_pair(std::move(fileName), h));				

				return true;
			}

			void RemoveHandle(const std::string &fileName)
			{
				m_mapHandles.erase(fileName);
			}

			bool IsEmpty() const
			{
				return m_mapHandles.empty();
			}

			bool IsHandleValid(std::string fileName) const
			{
				return m_mapHandles.find(fileName) != m_mapHandles.end();
			}

			void TryFireEvent(ldmonitor::fs::path path, std::string fileName, std::chrono::milliseconds time)
			{
				auto it = m_mapHandles.find(fileName);
				if (it == m_mapHandles.end())
					return;

				//too fast? Probably filesystem noise...
				if ((time - it->second.m_tLastTime) < dcclite::broker::FILE_WATCHER_IGNORE_TIME)
				{
					dcclite::Log::Trace("[FileWatcher::DirectoryWatcher::TryFireEvent] Ignoring event for {}, too soon", fileName);

					return;
				}
					
				it->second.m_tLastTime = time;
				it->second.m_pfnCallback(std::move(path), std::move(fileName));
			}

		private:	
			struct Handle
			{				
				Callback_t					m_pfnCallback;
				std::chrono::milliseconds	m_tLastTime;
			};

			std::map<std::string, Handle> m_mapHandles;
	};	

	class FileWatcherEvent : public dcclite::broker::EventHub::IEvent
	{
		public:
			FileWatcherEvent(EventTarget &target, const ldmonitor::fs::path &path, std::string fileName, std::chrono::milliseconds time) :
				IEvent(target),
				m_pthPath{ path },
				m_strFileName{ std::move(fileName) },
				m_tTime{ time }
			{
				//empty
			}

			void Fire() override
			{
				auto it = g_mapWatchers.find(m_pthPath);

				//removed? Ignore...
				if (it == g_mapWatchers.end())
					return;

				it->second.TryFireEvent(std::move(m_pthPath), std::move(m_strFileName), m_tTime);
			}

		private:
			ldmonitor::fs::path			m_pthPath;
			std::string					m_strFileName;
			std::chrono::milliseconds	m_tTime;
	};

	bool TryWatchFile(const dcclite::fs::path &fileName, const Callback_t &callback)
	{		
		auto filePath = fileName.parent_path();		

		auto it = g_mapWatchers.find(filePath);
		if (it == g_mapWatchers.end())
		{
			it = g_mapWatchers.insert(std::make_pair(filePath, DirectoryWatcher{})).first;			
			
			////////////////
			// 
			// Because the callback can be called by any thread, we forward all data to it, to avoid race conditions
			//
			//
			ldmonitor::Watch(filePath.string(), [](const ldmonitor::fs::path &path, std::string fileName, const uint32_t action, std::chrono::milliseconds time)
				{
					dcclite::broker::EventHub::PostEvent<FileWatcherEvent>(std::ref(g_clSentinel), path, std::move(fileName), time);
				}, 
				ldmonitor::MONITOR_ACTION_FILE_MODIFY
			);
		}

		if (!it->second.TryAddHandle(fileName.filename().string(), callback))
			return false;		

		return true;
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
			ldmonitor::Unwatch(filePath);			

			//Remove after unwatch, to make sure thread does not access it
			g_mapWatchers.erase(it);
		}		
	}	
}


