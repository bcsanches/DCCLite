// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ScriptService.h"

#include <sol/sol.hpp>

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>

#include "sys/Broker.h"
#include "sys/FileWatcher.h"
#include "sys/Project.h"
#include "sys/ServiceFactory.h"

#include "Proxies.h"

namespace dcclite::broker::shell::script
{
	const char *ScriptService::TYPE_NAME = "ScriptService";

	ScriptService::ScriptService(RName name, Broker &broker, const rapidjson::Value &params):
		Service(name, broker, params)
	{
		//empty
	}

	ScriptService::~ScriptService()
	{
		for (const auto &it : m_setScripts)
		{
			FileWatcher::UnwatchFile(it);
		}
	}
	
	void ScriptService::ConfigureLua()
	{
		auto path{ Project::GetFilePath("scripts") };
		path.append("autoexec.lua");

		if (!dcclite::fs::exists(path))
		{
			dcclite::Log::Warn("[ScriptService::Start] No autoexec.lua found, aborting.");

			return;
		}

		dcclite::Log::Trace("[ScriptService::Start] Opening lua libraries");

		m_clLua.open_libraries(sol::lib::base);
		m_clLua.open_libraries(sol::lib::string);

		dcclite::Log::Trace("[ScriptService::Start] Exporting functions");

		auto dccLiteTable = m_clLua["dcclite"].get_or_create<sol::table>();

		m_clLua.set_function(
			"run_script", 
			[this](const char *fileName)
			{
				auto path = Project::GetFilePath("scripts");
				path.append(fileName);				

				m_clLua.script_file(path.string());

				if (m_setScripts.find(path) != m_setScripts.end())
				{
					WatchFile(path);
					m_setScripts.insert(path);
				}				
			}
		);

		m_clLua.set_function(
			"log_error", 
			[](std::string_view msg)
			{
				dcclite::Log::Error("[ScriptService] [Lua] {}", msg);
			}
		);
		
		m_clLua.set_function(
			"log_info", 
			[](std::string_view msg)
			{
				dcclite::Log::Info("[ScriptService] [Lua] {}", msg);
			}
		);

		m_clLua.set_function(
			"log_trace", 
			[](std::string_view msg)
			{
				dcclite::Log::Trace("[ScriptService] [Lua] {}", msg);
			}
		);

		m_clLua.set_function(
			"log_warn", 
			[](std::string_view msg)
			{
				dcclite::Log::Warn("[ScriptService] [Lua] {}", msg);
			}
		);

		dcclite::Log::Trace("[ScriptService::Start] Exporting types");

		detail::AddTypes(m_clLua);
		
		dcclite::Log::Trace("[ScriptService::Start] Registering file monitor");

		WatchFile(path);		

		dcclite::Log::Trace("[ScriptService::Start] Looking up for known types");

		//
		//Export all known services
		m_rclBroker.VisitServices([&dccLiteTable, this](auto &item)
			{
				detail::TryCreateProxy(m_clLua, dccLiteTable, item);				

				return true;
			}
		);


		dcclite::Log::Trace("[ScriptService::Start] Running autoexec.lua");

		sol::protected_function_result r = m_clLua.safe_script_file(path.string());

		dcclite::Log::Info("[ScriptService::Start] done.");		
	}

	void ScriptService::WatchFile(const dcclite::fs::path &fileName)
	{
#if 1
		FileWatcher::TryWatchFile(fileName, [this](dcclite::fs::path path, std::string fileName)
			{
				dcclite::Log::Info("[ScriptService] [FileWatcher::Reload] Attempting to reload config: {}", fileName);

				try
				{
					//restart...
					this->Stop();
					this->Start();

					dcclite::Log::Info("[ScriptService] [FileWatcher::Reload] script system reloaded.");
				}
				catch (const std::exception &ex)
				{
					dcclite::Log::Error("[ScriptService] [FileWatcher::Reload] failed: {}", ex.what());
				}

			});
#endif
	}	

	void ScriptService::OnLoadFinished()
	{
		this->Start();
	}

	void ScriptService::OnUnload()
	{
		this->Stop();
	}

	void ScriptService::OnExecutiveChangeStart()
	{
		this->Stop();
	}

	void ScriptService::OnExecutiveChangeEnd()
	{
		this->Start();
	}

	void ScriptService::Start()
	{					
		if (m_fConfigured)
		{
			throw std::runtime_error("[ScriptService::Start] Attempt to start a already running service");
		}

		this->ConfigureLua();
		m_fConfigured = true;
	}

	void ScriptService::Stop()
	{
		dcclite::Log::Trace("[ScriptService::Stop] Closing lua");

		//Some services need to do a clenup first... let they know VM is going down
		detail::OnVmFinalize();

		//force destruction
		m_clLua = {};	

		m_fConfigured = false;

		dcclite::Log::Trace("[ScriptService::Stop] done");
	}

	void ScriptService::RegisterFactory()
	{
		//useless empty
	}

	static GenericServiceFactory<ScriptService> g_ServiceFactory;
}
