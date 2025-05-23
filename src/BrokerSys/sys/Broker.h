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

#include <rapidjson/rapidjson.h>

#include <dcclite/FileSystem.h>
#include <dcclite/Object.h>

#include "Service.h"

namespace dcclite
{
	class Clock;
}

namespace dcclite::broker::sys
{
	class Service;	

	class Broker: public FolderObject
	{
		public:
			explicit Broker(dcclite::fs::path projectPath);
			~Broker();

			Broker(const Broker &) = delete;

			Service *TryFindService(RName name);

			inline void VisitServices(Visitor_t visitor)
			{
				this->VisitChildren(visitor);
			}			

			const char *GetTypeName() const noexcept override
			{
				return "dcclite::Broker";
			}

			Service &ResolveRequirement(std::string_view requirement);

			void SignalExecutiveChangeStart();
			void SignalExecutiveChangeEnd();	

			IObject *AddChild(std::unique_ptr<Object> obj) override;
			void AddService(std::unique_ptr<Service> service);

		private:
			void LoadConfig();

			void LoadServices(const rapidjson::Value &servicesDataArray);		
	};
}
