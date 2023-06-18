// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DirectoryMonitor.h"

#include <array>
#include <mutex>
#include <map>

#include <fmt/format.h>

#include <spdlog/logger.h>

#include <Windows.h>

#include "LogUtils.h"

//https://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html

namespace dcclite::DirectoryMonitor
{	
	struct DirectoryMonitor
	{		
		OVERLAPPED						m_tOverlapped = { 0 };

		std::thread						m_thMonitorThread;

		HANDLE							m_hDirectory = INVALID_HANDLE_VALUE;	

		std::array<uint8_t, 1024 * 4>	m_arBuffer;

		dcclite::fs::path				m_pthPath;

		Callback_t						m_pfnCallback;

		uint32_t						m_u32Flags = 0;

		bool							m_fWaiting = false;
		bool							m_fCancel = false;
	};

	typedef std::map<fs::path, DirectoryMonitor> MapWatchers_t;
	
	static void RemoveWatcher(MapWatchers_t::iterator it, std::unique_lock<std::mutex> lock);

	struct State
	{
		std::mutex m_clLock;
		MapWatchers_t m_mapWatchers;

		~State()
		{
			while (!m_mapWatchers.empty())
			{
				std::unique_lock l{m_clLock};

				auto it = m_mapWatchers.begin();

				RemoveWatcher(it, std::move(l));
			}
		}
	};

	static State g_State;

	static inline DWORD Flags2Filter(const uint32_t flags) noexcept
	{			
		DWORD filter = (flags & MONITOR_ACTION_FILE_CREATE) ? (FILE_NOTIFY_CHANGE_FILE_NAME) : 0;

		filter = filter | ((flags & MONITOR_ACTION_FILE_DELETE) ? FILE_NOTIFY_CHANGE_FILE_NAME : 0);

		filter = filter | ((flags & MONITOR_ACTION_FILE_MODIFY) ? FILE_NOTIFY_CHANGE_LAST_WRITE : 0);

		filter = filter | ((flags & MONITOR_ACTION_FILE_RENAME_OLD_NAME) ? FILE_NOTIFY_CHANGE_FILE_NAME : 0);
		filter = filter | ((flags & MONITOR_ACTION_FILE_RENAME_NEW_NAME) ? FILE_NOTIFY_CHANGE_FILE_NAME : 0);

		return filter;
	}	
	
	uint32_t ReadActions2Flags(DWORD action) 
	{
		switch(action)
		{
			case FILE_ACTION_ADDED:
				return MONITOR_ACTION_FILE_CREATE;

			case FILE_ACTION_REMOVED:
				return MONITOR_ACTION_FILE_DELETE;

			case FILE_ACTION_RENAMED_OLD_NAME:
				return MONITOR_ACTION_FILE_RENAME_OLD_NAME;

			case FILE_ACTION_RENAMED_NEW_NAME:
				return MONITOR_ACTION_FILE_RENAME_NEW_NAME;

			case FILE_ACTION_MODIFIED:
				return MONITOR_ACTION_FILE_MODIFY;

			default:
				LogGetDefault()->error("[DirectoryMonitor] ReadActions2Flags unexpected value: {}", action);
				return 0;
		}
	}

	static bool RegisterWatcher(DirectoryMonitor &dirInfo)
	{
		return ReadDirectoryChangesW(
			dirInfo.m_hDirectory,
			&dirInfo.m_arBuffer[0],
			static_cast<DWORD>(dirInfo.m_arBuffer.size()),
			FALSE,
			Flags2Filter(dirInfo.m_u32Flags),
			nullptr,
			&dirInfo.m_tOverlapped,
			nullptr
		) != 0;		
	}

	static void ThreadProc(DirectoryMonitor &dirInfo)
	{		
		DWORD bytesRead;

		{
			std::lock_guard lock{g_State.m_clLock};

			if (dirInfo.m_fCancel)
			{
				LogGetDefault()->trace("[FileMonitor::ThreadProc] Thread cancelled before waiting for IO");
				return;
			}
				

			dirInfo.m_fWaiting = true;
		}

		for (;;)
		{			
			if (GetOverlappedResult(dirInfo.m_hDirectory, &dirInfo.m_tOverlapped, &bytesRead, TRUE) == 0)
			{				
				const auto ec = GetLastError();
				if (ec == ERROR_OPERATION_ABORTED)
				{
					LogGetDefault()->trace("[FileMonitor::ThreadProc] GetOverlappedResult aborted, thread exiting");					
				}
				else
				{
					LogGetDefault()->error("[FileMonitor::ThreadProc] GetOverlappedResult failed, ec: {}", ec);
				}					

				break;
			}							

			PFILE_NOTIFY_INFORMATION info;
			size_t offset = 0;
			do 
			{
				info = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(&dirInfo.m_arBuffer[0] + offset);
				offset += info->NextEntryOffset;

				auto action = ReadActions2Flags(info->Action);
				
				//any match?
				if (action & dirInfo.m_u32Flags) 
				{
					//https://stackoverflow.com/questions/3082620/convert-utf-16-to-utf-8

					//FileNameLength is size in bytes of the 16-bit Unicode string so, compute
					//the max number of chars that it could contain					
					int utf16len = info->FileNameLength / 2;

					int utf8len = WideCharToMultiByte(CP_UTF8, 0, info->FileName, utf16len, nullptr, 0, nullptr, nullptr);
					
					std::string utf8;

					//not sure why the constructor do not accept it...
					utf8.append(utf8len, '\0');					

					WideCharToMultiByte(
						CP_UTF8, 
						0, 
						info->FileName, 
						utf16len,
						&utf8[0], 
						utf8len, 
						0, 
						0
					);					

					dirInfo.m_pfnCallback(dirInfo.m_pthPath, std::move(utf8), action);
				}
			} while (info->NextEntryOffset != 0);

			std::unique_lock lock{g_State.m_clLock};

			if (dirInfo.m_fCancel)
			{
				return;
			}

			ResetEvent(dirInfo.m_tOverlapped.hEvent);

			if (!RegisterWatcher(dirInfo))
			{
				LogGetDefault()->error("[FileMonitor::ThreadProc] RegisterWatcher failed, ec: {}", GetLastError());

				return;
			}			
		}	
	}

	void Watch(const dcclite::fs::path &path, Callback_t callback, uint32_t flags)
	{		
		std::lock_guard lock{g_State.m_clLock};		

		auto it = g_State.m_mapWatchers.lower_bound(path);
		if ((it != g_State.m_mapWatchers.end()) && !(g_State.m_mapWatchers.key_comp()(path, it->first)))
		{
			throw std::invalid_argument(fmt::format("[WatchFile] Directory already has a watcher: {}", path.string()));
		}
		else
		{
			auto pathStr = path.string();
			HANDLE handle = CreateFile(
				pathStr.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
				nullptr,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				nullptr
			);

			if (handle == INVALID_HANDLE_VALUE)
			{
				throw std::invalid_argument(fmt::format("[WatchFile] Cannot open directory: {}", pathStr));
			}

			it = g_State.m_mapWatchers.emplace_hint(it, path, DirectoryMonitor{});

			auto &dirInfo = it->second;
			
			dirInfo.m_pthPath = path;
			dirInfo.m_u32Flags = flags;
			dirInfo.m_hDirectory = handle;
			dirInfo.m_pfnCallback = callback;

			dirInfo.m_tOverlapped.hEvent = CreateEventA(
				nullptr,
				TRUE,		//manual reset
				FALSE,		//start non signaled
				nullptr
			);

			if (dirInfo.m_tOverlapped.hEvent == nullptr)
			{
				CloseHandle(dirInfo.m_hDirectory);

				g_State.m_mapWatchers.erase(it);

				throw std::runtime_error(fmt::format("[WatchFile] Cannot create event for {} - ec {}", path.string(), GetLastError()));
			}

			if (!RegisterWatcher(dirInfo))
			{
				throw std::runtime_error(fmt::format("[WatchFile] ReadDirectoryChangesW failed for {} - ec {}", path.string(), GetLastError()));
			}

			it->second.m_thMonitorThread = std::thread{ [&dirInfo]() { ThreadProc(dirInfo); } };
		}
	}

	static void RemoveWatcher(MapWatchers_t::iterator it, std::unique_lock<std::mutex> lock)
	{
		if (std::this_thread::get_id() == it->second.m_thMonitorThread.get_id())
		{
			//called from the callback? - not supported
			throw std::logic_error("[[FileMonitor::UnwatchFile] Cannot remove watcher from the thread!");
		}

		it->second.m_fCancel = true;

		if (it->second.m_fWaiting)
		{
			CancelIoEx(it->second.m_hDirectory, &it->second.m_tOverlapped);

#if 0
			//make sure the thread 
			MsgWaitForMultipleObjectsEx(0, nullptr, 0, QS_ALLINPUT, MWMO_ALERTABLE);
			DWORD status = WaitForSingleObject(it->second.m_tOverlapped.hEvent, INFINITE);
			if (status == WAIT_FAILED)
			{
				LogGetDefault()->error("[FileMonitor::UnwatchFile] Wait failed for {}", it->second.m_pthPath.string());
			}
#endif
		}

		lock.unlock();

		//wait for the thread to die...
		it->second.m_thMonitorThread.join();

		CloseHandle(it->second.m_hDirectory);
		CloseHandle(it->second.m_tOverlapped.hEvent);

		lock.lock();

		g_State.m_mapWatchers.erase(it);
	}

	bool Unwatch(const dcclite::fs::path &path)
	{
		std::unique_lock lock{g_State.m_clLock};

		auto it = g_State.m_mapWatchers.find(path);

		if (it == g_State.m_mapWatchers.end())
			return false;

		RemoveWatcher(it, std::move(lock));

		return true;
	}

	namespace detail
	{
		std::optional<bool> IsThreadWaiting(const dcclite::fs::path &path)
		{
			std::unique_lock lock{g_State.m_clLock};

			auto it = g_State.m_mapWatchers.find(path);

			if (it == g_State.m_mapWatchers.end())
				return {};

			return it->second.m_fWaiting;
		}
	}
}
