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
#include <magic_enum.hpp>

#include "Broker.h"
#include "Decoder.h"
#include "DccLiteService.h"
#include "FileWatcher.h"
#include "Log.h"
#include "Project.h"
#include "RemoteDecoder.h"
#include "TurnoutDecoder.h"


class DecoderProxy
{	
	public:
		DecoderProxy(dcclite::broker::Decoder &decoder, dcclite::broker::Service &dccLiteService) :
			m_pclDecoder{ &decoder },
			m_clAddress{decoder.GetAddress()}
		{			
			m_slotObjectManagerConnection = dccLiteService.m_sigEvent.connect(&DecoderProxy::OnObjectManagerEvent, this);			

			dcclite::Log::Trace("[ScriptService] [DecoderProxy] [{}]: Created.", this->GetName());
		}

		~DecoderProxy()
		{
			dcclite::Log::Trace("[ScriptService] [DecoderProxy] [{}]: Destructor called", this->GetName());
		}

		inline const uint16_t GetAddress() const
		{			
			return m_clAddress.GetAddress();
		}

		inline std::string_view GetName() const
		{
			if (!m_pclDecoder)
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::GetName] [{}]: Decoder was destroyed, did you reload the config without it?", m_clAddress.GetAddress()));

			return this->GetDecoder().GetName();
		}

		inline bool IsActive() const
		{
			auto turnout = DynamicDecoderCast<dcclite::broker::RemoteDecoder>();

			return turnout->GetRemoteState() == dcclite::DecoderStates::ACTIVE;
		}

		void SetState(bool active)
		{
			auto decoder = DynamicDecoderCast<dcclite::broker::OutputDecoder>();			

			decoder->SetState(active ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE, "DecoderProxy");
		}

		inline bool IsInactive() const
		{
			return !this->IsActive();
		}

		void OnStateChange(sol::function callBack)
		{
			auto it = std::find(m_vStateChangeCallbacks.begin(), m_vStateChangeCallbacks.end(), callBack);
			if (it != m_vStateChangeCallbacks.end())
			{
				dcclite::Log::Warn("[ScriptService] [DecoderProxy::OnStateChange] [{}] Callback already registered", this->GetName());

				return;
			}

			m_vStateChangeCallbacks.push_back(callBack);

			if (!m_slotRemoteDecoderStateSyncConnection.connected())
				this->RegisterStateSyncCallback();
		}

	private:
		/**
		* Internal ref to the oficial decoder
		*
		* Note that if user changes the device config file, it will get unloaded and reloaded, this will invalidate the pointer and if the user removes the
		* decoder or the file fails to reload, it may stay null for a long time, so watch for a null pointer here....
		*
		*/
		dcclite::broker::Decoder *m_pclDecoder;
		dcclite::broker::DccAddress m_clAddress;

		sigslot::scoped_connection	m_slotObjectManagerConnection;
		sigslot::scoped_connection	m_slotRemoteDecoderStateSyncConnection;

		std::vector<sol::protected_function> m_vStateChangeCallbacks;

		template <typename T>
		inline T *DynamicDecoderCast()
		{
			auto decoder = dynamic_cast<T *>(&this->GetDecoder());
			if (!decoder)
			{
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::SetState] [{}]: Decoder is not a {output decoder{}!!!", this->GetName(), typeid(T).name()));
			}

			return decoder;
		}

		template <typename T>
		inline const T *DynamicDecoderCast() const
		{
			auto decoder = dynamic_cast<const T *>(&this->GetDecoder());
			if (!decoder)
			{
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::SetState] [{}]: Decoder is not a {output decoder{}!!!", this->GetName(), typeid(T).name()));
			}

			return decoder;
		}

		void OnRemoteDecoderStateSync(dcclite::broker::RemoteDecoder &decoder)
		{
			for (auto f : m_vStateChangeCallbacks)
			{				
				auto r = f.call(std::ref(*this));
				
				if (!r.valid())
				{
					sol::error err = r;

					dcclite::Log::Error("[ScriptService] [DecoderProxy::OnRemoteDecoderStateSync] Call failed with result: {} - {}", magic_enum::enum_name(r.status()), err.what());
				}
			}
		}

		void OnObjectManagerEvent(const dcclite::broker::ObjectManagerEvent &event);

		void RegisterStateSyncCallback()
		{
			auto remoteDecoder = this->DynamicDecoderCast<dcclite::broker::RemoteDecoder>();

			m_slotRemoteDecoderStateSyncConnection = remoteDecoder->m_sigRemoteStateSync.connect(&DecoderProxy::OnRemoteDecoderStateSync, this);
		}

		inline const dcclite::broker::Decoder &GetDecoder() const
		{
			if (!m_pclDecoder)
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::GetDecoder] [{}]: Decoder was destroyed, did you reload the config without it?", m_clAddress.GetAddress()));

			return *m_pclDecoder;
		}

		inline dcclite::broker::Decoder &GetDecoder()
		{
			if (!m_pclDecoder)
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::GetDecoder] [{}]: Decoder was destroyed, did you reload the config without it?", m_clAddress.GetAddress()));

			return *m_pclDecoder;
		}
};

void DecoderProxy::OnObjectManagerEvent(const dcclite::broker::ObjectManagerEvent &event)
{
	if (event.m_kType == dcclite::broker::ObjectManagerEvent::ITEM_CHANGED)
		return;	

	if (event.m_kType == dcclite::broker::ObjectManagerEvent::ITEM_DESTROYED)
	{
		if (event.m_pclItem != m_pclDecoder)
			return;

		dcclite::Log::Trace("[ScriptService] [DecoderProxy::OnObjectManagerEvent] [{}]: Decoder destroyed", this->GetName());

		m_slotRemoteDecoderStateSyncConnection.disconnect();
		m_pclDecoder = nullptr;
	}
	else if(!m_pclDecoder && (event.m_kType == dcclite::broker::ObjectManagerEvent::ITEM_CREATED))
	{
		auto createdDecoder = static_cast<dcclite::broker::Decoder *>(event.m_pclItem);

		if (createdDecoder->GetAddress() != m_clAddress)
			return;

		m_pclDecoder = createdDecoder;

		if (!m_vStateChangeCallbacks.empty())
		{
			this->RegisterStateSyncCallback();

			//make sure we notify because of possible state change
			OnRemoteDecoderStateSync(*this->DynamicDecoderCast<dcclite::broker::RemoteDecoder>());
		}

		dcclite::Log::Trace("[ScriptService] [DecoderProxy::OnObjectManagerEvent] [{}]: Decoder recreated", this->GetName());
	}
}

class DccLiteProxy
{
	public:
		DccLiteProxy(dcclite::broker::DccLiteService &service):
			m_rclService{service}
		{
			dcclite::Log::Trace("[ScriptService] [DccLiteProxy] [{}]: Created.", service.GetName());
		}

		DccLiteProxy(DccLiteProxy &&m) = default;

		DccLiteProxy(const DccLiteProxy &) = delete;	
		DccLiteProxy operator=(const DccLiteProxy &) = delete;

		~DccLiteProxy()
		{
			dcclite::Log::Trace("[ScriptService] [DccLiteProxy] [{}]: Destroyed.", m_rclService.GetName());
		}		

		DecoderProxy *OnIndexByName(std::string_view key, sol::this_state L);
		DecoderProxy *OnIndexByAddress(uint16_t key, sol::this_state L);

	private:
		dcclite::broker::DccLiteService &m_rclService;

		std::map<dcclite::broker::DccAddress, std::unique_ptr<DecoderProxy>> m_mapKnowDecoders;
};

DecoderProxy *DccLiteProxy::OnIndexByName(std::string_view key, sol::this_state L)
{
	const auto decoder = this->m_rclService.TryFindDecoder(key);

	if (!decoder)
	{
		dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

		return nullptr;
	}

	const auto decAddress = decoder->GetAddress();
	auto it = m_mapKnowDecoders.lower_bound(decAddress);

	if ((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
	{
		return it->second.get();
	}
	else
	{
		it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder, m_rclService));

		return it->second.get();
	}
}

DecoderProxy *DccLiteProxy::OnIndexByAddress(uint16_t key, sol::this_state L)
{
	const auto decAddress = dcclite::broker::DccAddress{ key };

	auto it = m_mapKnowDecoders.lower_bound(decAddress);

	if ((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
	{
		return it->second.get();
	}
	else
	{
		auto decoder = this->m_rclService.TryFindDecoder(decAddress);
		if (!decoder)
		{
			dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

			return nullptr;
		}

		it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder, m_rclService));

		return it->second.get();
	}
}


namespace dcclite::broker::ScriptService
{
	static sol::state g_clLua;	

	Broker *g_pclBroker = nullptr;
	const Project *g_pclProject = nullptr;

	static void WatchFile(const dcclite::fs::path &fileName);
	
	static void RunScripts()
	{
		auto path = g_pclProject->GetFilePath("scripts");
		path.append("autoexec.lua");

		if (!dcclite::fs::exists(path))
		{
			dcclite::Log::Info("[ScriptService::Start] No autoexec.lua found.");

			return;
		}

		g_clLua.open_libraries(sol::lib::base);

		auto dccLiteTable = g_clLua["dcclite"].get_or_create<sol::table>();

		g_clLua.set_function(
			"run_script", 
			[](const char *fileName)
			{
				auto path = g_pclProject->GetFilePath("scripts");
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

		g_clLua.new_usertype<DccLiteProxy>(
			"dcclite_service", 
			sol::no_constructor,
			sol::meta_function::index, 
			sol::overload(&DccLiteProxy::OnIndexByAddress, &DccLiteProxy::OnIndexByName)
		);

		g_clLua.new_usertype<DecoderProxy>(
			"decoder", sol::no_constructor,
			"address", sol::property(&DecoderProxy::GetAddress),
			"thrown", sol::property(&DecoderProxy::IsActive),
			"closed", sol::property(&DecoderProxy::IsInactive),
			"active", sol::property(&DecoderProxy::IsActive),
			"inactive", sol::property(&DecoderProxy::IsInactive),
			"set_state", &DecoderProxy::SetState,
			"on_state_change", &DecoderProxy::OnStateChange
		);

		//
		//Export all services
		auto servicesEnumerator = g_pclBroker->GetServicesEnumerator();

		while (servicesEnumerator.MoveNext())
		{
			auto service = servicesEnumerator.GetCurrent<Service>();
			
			if (auto dccLiteService = dynamic_cast<DccLiteService *>(service))
			{
				dccLiteTable[dccLiteService->GetName()] = DccLiteProxy{ *dccLiteService };
			}
			else
			{
				dccLiteTable[service->GetName()] = service;
			}
		}				

		WatchFile(path);

		auto r = g_clLua.safe_script_file(path.string());
	}


	static void WatchFile(const dcclite::fs::path &fileName)
	{
#if 1
		FileWatcher::TryWatchFile(fileName, [](dcclite::fs::path path, std::string fileName)
			{
				dcclite::Log::Info("[ScriptService] [FileWatcher::Reload] Attempting to reload config: {}", fileName);

				try
				{					
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

	void Start(Broker &broker, const Project &project)
	{				
		g_pclBroker = &broker;
		g_pclProject = &project;
					
		RunScripts();		
	}
}
