// Copyright (C) 2023 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <set>
#include <string_view>

#include <sol/sol.hpp>

#include <dcclite/FileSystem.h>

#include "sys/Service.h"

namespace dcclite::broker::sys
{	
	class Broker;
}

namespace dcclite::broker::shell::script
{
	class ScriptService: public sys::Service, public sys::IPostLoadService, public sys::IExecutiveClientService
	{
		public:
			static void RegisterFactory();

			ScriptService(RName name, sys::Broker &broker, const rapidjson::Value &params);
			~ScriptService() override;

			void OnLoadFinished() override;
			void OnUnload() override;

			void OnExecutiveChangeStart();
			void OnExecutiveChangeEnd();

			static const char *TYPE_NAME;		

		private:
			void ConfigureLua();
			void WatchFile(const dcclite::fs::path &fileName);

			void Start();
			void Stop();

		private:
			sol::state						m_clLua;
			std::set< dcclite::fs::path>	m_setScripts;

			bool							m_fConfigured = false;
	};
}
