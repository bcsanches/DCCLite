// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "EEPROM.h"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>

#include <spdlog/logger.h>

#include "EEPROMLib.h"
#include "FileSystem.h"
#include "PathUtils.h"

#include <Log.h>

EEPROMImpl EEPROM;

#define EEPROM_SIZE 2048

bool g_fDirty = false;
static std::array<std::uint8_t, EEPROM_SIZE> g_Data;
static std::array<std::uint8_t, EEPROM_SIZE> g_DataBackup;

static std::string g_strRomFileName;
static std::string g_strRomTempFileName;
static std::string g_strRomBackup;

static std::condition_variable g_WorkerMonitor;
static std::condition_variable g_MainMonitor;
static std::mutex g_WorkerMutex;

static std::thread g_thWorker;

static volatile bool g_fWorkerStop = false;
static volatile bool g_fDataReady = false;

void EEPROMImpl::get(size_t pos, void *ptr, size_t len)
{
	if (ptr == nullptr) 
	{
		throw std::invalid_argument("destination cannot be null");
	}

	if (pos + len > EEPROM_SIZE)
	{
		throw std::out_of_range("out of bounds");
	}

	memcpy(ptr, &g_Data[pos], len);
}

void EEPROMImpl::put(size_t pos, const void *ptr, size_t len)
{
	if (ptr == nullptr)
	{
		throw std::invalid_argument("destination cannot be null");
	}

	if (pos + len > EEPROM_SIZE)
	{
		throw std::out_of_range("out of bounds");
	}

	memcpy(&g_Data[pos], ptr, len);
	g_fDirty = true;
}

unsigned char EEPROMImpl::read(size_t pos)
{
	return g_Data.at(pos);
}

size_t EEPROMImpl::length()
{
	return g_Data.max_size();
}

namespace ArduinoLib::detail
{
	
	/**
		We try the best to keep the rom state consistent on disk.

		When saving, we first write new content to a temporary file.

		After the file is done, we rename existing rom file to a backup name. 

		Then we rename the the new rom file to a valid rom state file

		Last, delete the rom backup.

		--------------------------			
	*/

	static bool TrySaveRomState()
	{				
		dcclite::fs::remove(g_strRomBackup);

		FILE *fp = fopen(g_strRomTempFileName.c_str(), "wb");
		if (fp == nullptr)
			return false;			

		fwrite(&g_DataBackup[0], 1, EEPROM_SIZE, fp);

		fclose(fp);

		std::error_code ec;
		
		dcclite::fs::rename(g_strRomFileName, g_strRomBackup, ec);
		dcclite::fs::rename(g_strRomTempFileName, g_strRomFileName, ec);

		dcclite::fs::remove(g_strRomBackup);

		return true;
	}

	static void WorkerThread()
	{
		std::unique_lock<std::mutex> lck(g_WorkerMutex, std::defer_lock);
		while(!g_fWorkerStop)
		{			
			lck.lock();
			
			g_WorkerMonitor.wait(lck, [] 
			{
				dcclite::Log::Trace("RomWorkerThread waiting [WorkerStop {}] [DataReady {}]", g_fWorkerStop, g_fDataReady);

				return g_fWorkerStop ? true : g_fDataReady;
			});

			if (!g_fDataReady)
			{
				dcclite::Log::Trace("RomWorkerThread exiting");
				break;
			}
			
			dcclite::Log::Info("RomWorkerThread working");

			TrySaveRomState();

			g_fDataReady = false;			

			dcclite::Log::Info("RomWorkerThread done");
			
			lck.unlock();			
			g_MainMonitor.notify_one();
		}

		g_MainMonitor.notify_one();
	}

	static void RequestRomStateSave()
	{
		{
			std::unique_lock<std::mutex> lck(g_WorkerMutex);

			g_MainMonitor.wait(lck, []
			{
				//g_Log->trace("RequestRomStateSave waiting[{}]", g_fDataReady);
				return !g_fDataReady;
			});

			memcpy(&g_DataBackup[0], &g_Data[0], g_Data.size());
			g_fDirty = false;
			g_fDataReady = true;
		}		
				
		g_WorkerMonitor.notify_one();
	}

	static void WaitSaveWorker()
	{
		std::unique_lock lck(g_WorkerMutex);

		g_MainMonitor.wait(lck, [] {return !g_fDataReady; });
	}

	static void Clear()
	{
		memset(&g_Data[0], 0, g_Data.size());
		memset(&g_DataBackup[0], 0, g_DataBackup.size());
	}

	static void ReadRomData(FILE *fp)
	{
		fread(&g_Data[0], 1, g_Data.size(), fp);
	}

	/**
	When loading back rom state, we must check for a inconsistent state

	1 - First, check if the rom state file exists, if yes load it

	If no rom state, check for a backup file if yes, check for rom temporary, if exists, rename it and goevilto 1.
		- that should not happen, but if a backup and no temporary rom, rename backup and goevilto 1

	if no backup file, sorry, no rom state... clear it

	*/
	static bool TryLoadRomState()
	{
		dcclite::Log::Info("TryLoadRomState: trying to load Rom {}", g_strRomFileName);

		if (dcclite::fs::exists(g_strRomBackup) && dcclite::fs::exists(g_strRomTempFileName))
		{
			dcclite::Log::Warn("TryLoadRomState: found backup {}, restoring it", g_strRomTempFileName);

			dcclite::fs::remove(g_strRomFileName);
			
			dcclite::fs::rename(g_strRomTempFileName, g_strRomFileName);
			dcclite::fs::remove(g_strRomBackup);

			dcclite::Log::Warn("TryLoadRomState: backup ready");
		}

		FILE *fp = fopen(g_strRomFileName.c_str(), "rb");
		if (fp == nullptr)
		{
			dcclite::Log::Warn("TryLoadRomState: failed to open Rom {}", g_strRomFileName);

			return false;
		}		

		ReadRomData(fp);

		fclose(fp);

		dcclite::Log::Info("TryLoadRomState: ok");
		return true;
	}

	void RomSetupModule(std::string_view moduleName)
	{
		//first time?
		if (!g_thWorker.joinable())
		{			
			dcclite::Log::Info("RomSetupModule: started worker thread");

			//start the thread
			g_fWorkerStop = false;			

			g_thWorker = std::move(std::thread(WorkerThread));			
		}

		if (g_fDirty)
		{
			dcclite::Log::Info("RomSetupModule: Updating module and requesting to save old rom");

			RequestRomStateSave();			
		}

		//make sure thread is not busy
		WaitSaveWorker();

		auto appPath = dcclite::PathUtils::GetAppFolder();
		appPath.append("Emulator");
		dcclite::fs::create_directories(appPath);

		g_strRomFileName = (appPath / dcclite::fs::path(moduleName).replace_extension(".rom")).string();

		g_strRomTempFileName = g_strRomFileName;
		g_strRomTempFileName += ".tmp";

		g_strRomBackup = g_strRomFileName;
		g_strRomBackup += ".bkp";
		
		TryLoadRomState();
	}	

	void RomFinalize()
	{
		if (g_thWorker.joinable())
		{	
			{
				std::lock_guard lck(g_WorkerMutex);
				g_fWorkerStop = true;
			}

			dcclite::Log::Trace("RomFinalize waiting worker thread");

			g_WorkerMonitor.notify_one();
			WaitSaveWorker();

			dcclite::Log::Trace("RomFinalize joining worker");

			g_WorkerMonitor.notify_one();
			g_thWorker.join();

			dcclite::Log::Trace("RomFinalize worker finished");

			if (g_fDataReady)
			{
				dcclite::Log::Info("RomFinalize g_fDataReady, saving last state on main thread");

				TrySaveRomState();
			}

			dcclite::Log::Info("RomFinalize done");
		}
	}

	void RomAfterLoop()
	{		
		if(g_fDirty)
			RequestRomStateSave();
	}
}

