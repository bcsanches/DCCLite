// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <iostream>

#include <thread>

#include <spdlog/logger.h>

#include <dcclite/Console.h>
#include <dcclite/dcclite.h>
#include <dcclite/FileSystem.h>
#include <dcclite/Log.h>
#include <dcclite/NetMessenger.h>
#include <dcclite/PathUtils.h>

using namespace dcclite;

using namespace std::chrono_literals;

#ifdef WIN32
static const char *gBrokerExecutableName = "Broker.exe";
#else
static const char *gBrokerExecutableName = "Broker";
#endif


#ifdef WIN32

#include <Windows.h>

class Process
{
	public:
		Process(const fs::path& brokerPath, int argc, char** argv)
		{
			std::stringstream cmdLine;

			cmdLine << brokerPath << ' ';

			for (int i = 1; i < argc; ++i)
			{
				cmdLine << argv[i];
				cmdLine << ' ';
			}

			STARTUPINFO si;

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);

			ZeroMemory(&mInfo, sizeof(mInfo));		

			// Start the child process. 
			if (!CreateProcess(
				NULL,										// No module name (use command line)
				const_cast<char*>(cmdLine.str().c_str()),	// Command line
				NULL,										// Process handle not inheritable
				NULL,										// Thread handle not inheritable
				FALSE,										// Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,							// No creation flags
				NULL,										// Use parent's environment block
				NULL,										// Use parent's starting directory 
				&si,										// Pointer to STARTUPINFO structure
				&mInfo)										// Pointer to PROCESS_INFORMATION structure
				)
			{				
				throw std::runtime_error(fmt::format("[LAUNCHER] Cannot start Broker process, failed: {}", GetLastError()));
			}
		}

		bool IsRunning() const
		{
			auto r = WaitForSingleObject(mInfo.hProcess, 0);
			switch (r)
			{
				case WAIT_TIMEOUT:
					return true;

				case WAIT_OBJECT_0:
					return false;

				default:
					throw std::runtime_error(fmt::format("[LAUNCHER] Unexpected wait result: {}", r));
			}
		}

		~Process()
		{
			CloseHandle(mInfo.hThread);
			CloseHandle(mInfo.hProcess);
		}

	private:
		PROCESS_INFORMATION mInfo;
};

#else

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

class Process
{
	public:
		Process(const fs::path &brokerPath, int argc, char** argv):
			m_pid(fork())
		{							
			if (m_pid == -1)
			{
				throw std::runtime_error(fmt::format("[LAUNCHER] fork failed: {}", errno));
			}

			if (m_pid != 0)
			{
				//we are the parent

				return;
			}

			std::vector<const char *> newArgs;
			newArgs.reserve(argc + 1);

			newArgs.push_back(gBrokerExecutableName);
			
			for (int i = 1; i < argc; ++i)
				newArgs.push_back(argv[i]);

			newArgs.push_back(nullptr);

			if (execv(brokerPath.string().c_str(), const_cast<char **>(&newArgs[0])) == -1)
			{
				throw std::runtime_error(fmt::format("[LAUNCHER] execv failed: {}", errno));
			}

			//never reached
		}

		bool IsRunning() const
		{
			//send a blank signal to see if process is still there
			return kill(m_pid, 0) == 0;
		}

	private:
		pid_t m_pid;
};

#endif


int main(int argc, char **argv)
{
	dcclite::Init("Launcher", "Launcher.log");		

	fs::path brokerPath = fs::path{ gBrokerExecutableName };

#ifdef _DEBUG
	ConsoleTryMakeNice();
#endif

	dcclite::Log::Info("[LAUNCHER] Looking for Broker main file at {}", fs::current_path().string());	

	if (!fs::exists(brokerPath))
	{
		dcclite::Log::Warn("[LAUNCHER] Broker executable {} not found at running path", brokerPath.string());

		brokerPath = fs::path{ argv[0] };

		brokerPath.replace_filename(gBrokerExecutableName);

		if (!fs::exists(brokerPath))
		{
			dcclite::Log::Critical("[LAUNCHER] Broker executable {} not found, cannot continue", brokerPath.string());

			return -1;
		}
	}

	dcclite::Log::Info("[LAUNCHER] Broker executable found at {}", brokerPath.string());

	try
	{
		Process broker{ brokerPath, argc, argv };

		//wait for the process to go stable
		for(;;)
		{
			if (!broker.IsRunning())
			{
				dcclite::Log::Critical("[LAUNCHER] Broker process appears to be dead");

				return -1;
			}

			std::this_thread::sleep_for(1000ms);

			dcclite::Log::Info("[LAUNCHER] Trying to connect to Broker");

			Socket socket{};

			if (!socket.Open(0, Socket::Type::STREAM))
			{
				dcclite::Log::Critical("[LAUNCHER] Cannot create socket");

				return -1;
			}

			if (!socket.StartConnection(NetworkAddress(127, 0, 0, 1, 4190)))
			{
				dcclite::Log::Error("[LAUNCHER] Cannot connect to Broker");

				continue;
			}

			while (socket.GetConnectionProgress() == Socket::Status::WOULD_BLOCK)
			{
				std::this_thread::sleep_for(100ms);
			}

			auto info = socket.GetConnectionProgress();
			if (info == Socket::Status::OK)
			{
				break;
			}
		}
	}
	catch (std::exception& ex)
	{
		dcclite::Log::Critical("[LAUNCHER] Critical error: {}", ex.what());

		return -1;
	}	

	dcclite::Log::Info("[LAUNCHER] Broker is running, launcher exiting...");

	return 0;
}
