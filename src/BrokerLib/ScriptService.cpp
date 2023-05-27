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

#include "Broker.h"
#include "DccLiteService.h"
#include "Log.h"
#include "Project.h"

class DecoderProxy
{
	private:
		dcclite::broker::Decoder &m_rclDecoder;

	public:
		DecoderProxy(dcclite::broker::Decoder &decoder) :
			m_rclDecoder{ decoder }
		{
			//empty
		}

		inline const uint16_t GetAddress() const noexcept
		{
			return m_rclDecoder.GetAddress().GetAddress();
		}

		~DecoderProxy()
		{
			int a = 1;
			++a;
		}
};

class DccLiteProxy
{
	public:
		DccLiteProxy(dcclite::broker::DccLiteService &service):
			m_rclService{service}
		{
			//empty
		}

		DccLiteProxy(DccLiteProxy &&m) = default;

		DccLiteProxy(const DccLiteProxy &) = delete;	
		DccLiteProxy operator=(const DccLiteProxy &) = delete;

		~DccLiteProxy()
		{
			int a = 0;
			++a;
		}		

		sol::object OnIndexByName(std::string_view key, sol::this_state L)
		{			
			const auto decoder = this->m_rclService.TryFindDecoder(key);

			if (!decoder)
			{
				dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

				return sol::lua_nil;
			}

			const auto decAddress = decoder->GetAddress();
			auto it = m_mapKnowDecoders.lower_bound(decAddress);

			if((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
				return sol::make_object<DecoderProxy *>(L, it->second.get());
			else
			{
				it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder));

				return sol::make_object<DecoderProxy *>(L, it->second.get());
			}			
		}

		sol::object OnIndexByAddress(uint16_t key, sol::this_state L)
		{
			const auto decAddress = dcclite::broker::DccAddress{ key };
						
			auto it = m_mapKnowDecoders.lower_bound(decAddress);

			if ((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
				return sol::make_object<DecoderProxy *>(L, it->second.get());
			else
			{
				auto decoder = this->m_rclService.TryFindDecoder(decAddress);
				if (!decoder)
				{
					dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

					return sol::lua_nil;
				}

				it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder));

				return sol::make_object<DecoderProxy *>(L, it->second.get());
			}
		}

	private:
		dcclite::broker::DccLiteService &m_rclService;

		std::map<dcclite::broker::DccAddress, std::unique_ptr<DecoderProxy>> m_mapKnowDecoders;
};

namespace dcclite::broker::ScriptService
{
	static sol::state g_clLua;
	sol::environment g_clSandbox{g_clLua, sol::create};

	Broker *g_pclBroker = nullptr;

	void Start(Broker &broker, const Project &project)
	{				
		g_pclBroker = &broker;

		auto path = project.GetFilePath("scripts");
		path.append("autoexec.lua");

		if (!dcclite::fs::exists(path))
			return;

		g_clLua.open_libraries(sol::lib::base);		

		auto dccLiteTable = g_clLua["dcclite"].get_or_create<sol::table>();

		g_clLua.new_usertype<DccLiteProxy>(
			"dcclite_service", sol::no_constructor,			
			sol::meta_function::index, sol::overload(&DccLiteProxy::OnIndexByAddress, &DccLiteProxy::OnIndexByName)
		);

		g_clLua.new_usertype<DecoderProxy>(
			"decoder",		sol::no_constructor,			
			"address",		sol::property(&DecoderProxy::GetAddress)
		);

		//
		//Export all services
		auto servicesEnumerator = broker.GetServicesEnumerator();

		while (servicesEnumerator.MoveNext())
		{
			auto service = servicesEnumerator.GetCurrent<Service>();

			auto dccLiteService = dynamic_cast<DccLiteService *>(service);
			if (dccLiteService)
			{
				dccLiteTable[dccLiteService->GetName()] = DccLiteProxy{ *dccLiteService };
			}
			else
			{
				dccLiteTable[service->GetName()] = service;
			}			
		}

		g_clSandbox["print"] = g_clLua["print"];
		g_clSandbox["dcclite"] = dccLiteTable;
		g_clSandbox["collectgarbage"] = g_clLua["collectgarbage"];

		//g_clSandbox["dofile"] = g_clLua["dofile"];

		g_clSandbox.set_function("dofile", [&project](const char *fileName)
			{
				auto path = project.GetFilePath("scripts");
				path.append(fileName);

				g_clLua.safe_script_file(path.string());
			}
		);

		auto r = g_clLua.safe_script_file(path.string(), g_clSandbox);

		return;
	}
}
