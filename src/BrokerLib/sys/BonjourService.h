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

#include "Service.h"

namespace dcclite::broker
{ 
	enum class NetworkProtocol
	{
		TCP,
		UDP
	};

	constexpr auto BONJOUR_SERVICE_NAME = "bonjour";

	class BonjourService: public Service
	{	
		public:
			static void RegisterFactory();

			static const char *TYPE_NAME;

			BonjourService(RName name, Broker &broker, const rapidjson::Value &params);
		
			~BonjourService() override = default;

			virtual void Register(std::string_view instanceName, std::string_view serviceName, const NetworkProtocol protocol, const uint16_t port, const uint32_t ttl) = 0;

			//
			//
			//			

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}
	};
}
