// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ScriptSystem.h"

#include <sol/sol.hpp>

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>

#include "../sys/Broker.h"
#include "../sys/FileWatcher.h"
#include "../sys/Project.h"


namespace dcclite::broker::ScriptSystem
{
	static sol::state g_clLua;	

	Broker *g_pclBroker = nullptr;	

	std::vector<IScriptSupport *> g_vecRegisteredServices;

	static void WatchFile(const dcclite::fs::path &fileName);
	
	static void RunScripts()
	{
		auto path{ Project::GetFilePath("scripts") };
		path.append("autoexec.lua");

		if (!dcclite::fs::exists(path))
		{
			dcclite::Log::Warn("[ScriptService::Start] No autoexec.lua found, aborting.");

			return;
		}

		dcclite::Log::Trace("[ScriptService::Start] Opening lua libraries");

		g_clLua.open_libraries(sol::lib::base);
		g_clLua.open_libraries(sol::lib::string);

		dcclite::Log::Trace("[ScriptService::Start] Exporting functions");

		auto dccLiteTable = g_clLua["dcclite"].get_or_create<sol::table>();

		g_clLua.set_function(
			"run_script", 
			[](const char *fileName)
			{
				auto path = Project::GetFilePath("scripts");
				path.append(fileName);

				WatchFile(path);

				g_clLua.script_file(path.string());
			}
		);

		g_clLua.set_function(
			"log_error", 
			[](std::string_view msg)
			{
				dcclite::Log::Error("[ScriptService] [Lua] {}", msg);
			}
		);
		
		g_clLua.set_function(
			"log_info", 
			[](std::string_view msg)
			{
				dcclite::Log::Info("[ScriptService] [Lua] {}", msg);
			}
		);

		g_clLua.set_function(
			"log_trace", 
			[](std::string_view msg)
			{
				dcclite::Log::Trace("[ScriptService] [Lua] {}", msg);
			}
		);

		g_clLua.set_function(
			"log_warn", 
			[](std::string_view msg)
			{
				dcclite::Log::Warn("[ScriptService] [Lua] {}", msg);
			}
		);

		dcclite::Log::Trace("[ScriptService::Start] Exporting services");

		//
		//Export all services
		g_pclBroker->VisitServices([&dccLiteTable](auto &item)
			{				
				if (auto *scripter = dynamic_cast<IScriptSupport *>(&item))
				{
					dcclite::Log::Trace("[ScriptService::Start] Registering proxy for service {}", item.GetName());

					scripter->IScriptSupport_RegisterProxy(dccLiteTable);
					g_vecRegisteredServices.push_back(scripter);
				}				

				return true;
			}
		);

		dcclite::Log::Trace("[ScriptService::Start] Services exported");

		//
		//Notify whoever wants to register something that VM is ready to start running
		for (auto it : g_vecRegisteredServices)
			it->IScriptSupport_OnVMInit(g_clLua);

		dcclite::Log::Trace("[ScriptService::Start] Registering file monitor");

		WatchFile(path);		

		dcclite::Log::Trace("[ScriptService::Start] Running autoexec.lua");

		sol::protected_function_result r = g_clLua.safe_script_file(path.string());

		dcclite::Log::Info("[ScriptService::Start] done.");
	}

	static void WatchFile(const dcclite::fs::path &fileName)
	{
#if 1
		FileWatcher::TryWatchFile(fileName, [](dcclite::fs::path path, std::string fileName)
			{
				dcclite::Log::Info("[ScriptService] [FileWatcher::Reload] Attempting to reload config: {}", fileName);

				try
				{		
					//
					//Before destroying VM, let others know
					for (auto it : g_vecRegisteredServices)
						it->IScriptSupport_OnVMFinalize(g_clLua);

					g_vecRegisteredServices.clear();

					g_clLua = sol::state{};					

					RunScripts();

					dcclite::Log::Info("[ScriptService] [FileWatcher::Reload] script system reloaded.");
				}
				catch (const std::exception &ex)
				{
					dcclite::Log::Error("[ScriptService] [FileWatcher::Reload] failed: {}", ex.what());
				}

			});
#endif
	}	

	void Start(Broker &broker)
	{				
		g_pclBroker = &broker;		
					
		RunScripts();		
	}

	void Stop()
	{
		dcclite::Log::Trace("[ScriptService::Stop] Notifying scripters");

		for (auto it : g_vecRegisteredServices)
			it->IScriptSupport_OnVMFinalize(g_clLua);		

		dcclite::Log::Trace("[ScriptService::Stop] Closing lua");

		//force destruction
		g_clLua = {};

		g_vecRegisteredServices.clear();
		g_pclBroker = nullptr;

		dcclite::Log::Trace("[ScriptService::Stop] done");
	}
}
