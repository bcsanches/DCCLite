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

#include "../dcc/DccAddress.h"

#include "../sys/Service.h"

namespace dcclite::broker
{ 
	class ILoconetSlot;

	class IThrottle
	{
		public:
			virtual ~IThrottle()
			{
				//emtpy
			}

			virtual void OnSpeedChange() = 0;
			virtual void OnForwardChange() = 0;

			virtual void OnFunctionChange(const uint8_t begin, const uint8_t end) = 0;

			virtual void OnEmergencyStop() = 0;

			virtual void AddSlave(const ILoconetSlot &slot) = 0;
			virtual void RemoveSlave(const ILoconetSlot &slot) = 0;

			virtual bool HasSlaves() const noexcept = 0;
	};


	class ThrottleService: public Service
	{	
		public:
			ThrottleService(RName name, Broker &broker, const rapidjson::Value& params, const Project& project);
		
			~ThrottleService() override
			{
				//empty
			}

			//
			// Main interface
			//

			virtual IThrottle &CreateThrottle(const ILoconetSlot &owner) = 0;

			virtual void ReleaseThrottle(IThrottle &throttle) = 0;

			//
			//
			//
		
			static std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &params, const Project &project);

			const char *GetTypeName() const noexcept override
			{
				return "ThrottleService";
			}
	};
}
