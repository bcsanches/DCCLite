// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <vector>

#include "Service.h"
#include "Thinker.h"

#include "Socket.h"

namespace dcclite::broker
{

	class TerminalClient;

	class TerminalService : public Service
	{
		private:		
			dcclite::Socket m_clSocket;

			std::vector <std::unique_ptr<TerminalClient>> m_vecClients;

			Thinker m_tThinker;

		private:
			void Think(const dcclite::Clock::TimePoint_t tp);

		public:
			TerminalService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			virtual ~TerminalService();			

			const char *GetTypeName() const noexcept override { return "TerminalService"; }
	};
}