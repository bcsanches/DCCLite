#include "EEPROM.h"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <thread>
#include <mutex>

#include "EEPROMLib.h"

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

static bool g_fWorkerStop = false;
static bool g_fDataReady = false;

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
		std::filesystem::remove(g_strRomBackup);

		FILE *fp = fopen(g_strRomTempFileName.c_str(), "wb");
		if (fp == nullptr)
			return false;			

		fwrite(&g_DataBackup[0], 1, EEPROM_SIZE, fp);

		fclose(fp);

		std::error_code ec;
		
		std::filesystem::rename(g_strRomFileName, g_strRomBackup, ec);
		std::filesystem::rename(g_strRomTempFileName, g_strRomFileName, ec);

		std::filesystem::remove(g_strRomBackup);

		return true;
	}

	static void WorkerThread()
	{
		std::unique_lock<std::mutex> lck(g_WorkerMutex, std::defer_lock);
		for (;;)
		{			
			lck.lock();
			
			g_WorkerMonitor.wait(lck, [] 
			{
				//g_Log->trace("RomWorkerThread waiting[{}]", !g_fDataReady);

				return g_fDataReady; 
			});

			if (g_fWorkerStop)
				return;			
			
			//g_Log->info("RomWorkerThread working");

			TrySaveRomState();

			g_fDataReady = false;			

			//g_Log->info("RomWorkerThread done");
			
			lck.unlock();			
			g_MainMonitor.notify_one();
		}
	}

	static void RequestRomStateSave()
	{
		std::unique_lock lck(g_WorkerMutex);
		
		g_MainMonitor.wait(lck, [] 
		{
			//g_Log->trace("RequestRomStateSave waiting[{}]", g_fDataReady);
			return !g_fDataReady; 
		});

		memcpy(&g_DataBackup[0], &g_Data[0], g_Data.size());
		g_fDirty = false;	
		g_fDataReady = true;
		
		lck.unlock();
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
		if (std::filesystem::exists(g_strRomBackup) && std::filesystem::exists(g_strRomTempFileName))
		{
			std::filesystem::remove(g_strRomFileName);
			
			std::filesystem::rename(g_strRomTempFileName, g_strRomFileName);
			std::filesystem::remove(g_strRomBackup);			
		}

		FILE *fp = fopen(g_strRomFileName.c_str(), "rb");
		if (fp == nullptr)
		{
			return false;
		}		

		ReadRomData(fp);

		fclose(fp);

		return true;
	}

	void RomSetupModule(std::string_view moduleName)
	{
		//first time?
		if (!g_thWorker.joinable())
		{
			//start the thread
			g_fWorkerStop = false;			

			g_thWorker = std::move(std::thread(WorkerThread));

			//g_Log = log;

			//g_Log->info("RomSetupModule: started worker thread");
		}

		if (g_fDirty)
		{
			//g_Log->info("RomSetupModule: Updating module and requesting to save old rom");

			RequestRomStateSave();			
		}

		//make sure thread is not busy
		WaitSaveWorker();

		g_strRomFileName = std::filesystem::path(moduleName).replace_extension(".rom").string();

		g_strRomTempFileName = g_strRomFileName;
		g_strRomTempFileName += ".tmp";

		g_strRomBackup = g_strRomFileName;
		g_strRomBackup += ".bkp";
		
		TryLoadRomState();
	}	

	void RomAfterLoop()
	{		
		if(g_fDirty)
			RequestRomStateSave();
	}
}

