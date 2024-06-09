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

#include <map>
#include <memory>
#include <string>

#include "Object.h"
#include "Project.h"
#include "Service.h"

namespace dcclite
{
	class Clock;
}

namespace dcclite::broker
{
	class Service;
	class TerminalCmdHost;

	class Broker: public FolderObject
	{
		public:
			explicit Broker(dcclite::fs::path projectPath);

			Broker(const Broker &) = delete;

			Service *TryFindService(RName name);

			inline void VisitServices(Visitor_t visitor)
			{
				m_pServices->VisitChildren(visitor);
			}

			TerminalCmdHost *GetTerminalCmdHost()
			{
				return m_pclTerminalCmdHost;
			}

			const char *GetTypeName() const noexcept override
			{
				return "dcclite::Broker";
			}

			Service &ResolveRequirement(const char *requirement);

		private:				
			dcclite::FolderObject	*m_pServices;

			TerminalCmdHost			*m_pclTerminalCmdHost = nullptr;

			Project m_clProject;

		private:
			void LoadConfig();	
	};
}
