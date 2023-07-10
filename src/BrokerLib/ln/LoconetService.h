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

#include <Packet.h>

#include "../sys/Service.h"

#include "../dcc/DccAddress.h"


namespace dcclite::broker
{ 
	typedef dcclite::BitPack<32> LoconetSlotFunctions_t;

	class ILoconetSlot
	{
		public:
			virtual ~ILoconetSlot() = default;

			ILoconetSlot(const ILoconetSlot &rhs) = delete;
			ILoconetSlot(ILoconetSlot &&rhs) = delete;

			ILoconetSlot &operator=(const ILoconetSlot &rhs) = delete;
			ILoconetSlot &&operator=(const ILoconetSlot &&rhs) = delete;

			uint8_t GetId() const noexcept
			{
				return m_uId;
			}

			const dcclite::broker::DccAddress &GetLocomotiveAddress() const noexcept
			{
				return m_tLocomotiveAddress;
			}

			bool IsForwardDir() const noexcept
			{
				return m_fForward;
			}

			uint8_t GetSpeed() const noexcept
			{
				return m_uSpeed;
			}

			const LoconetSlotFunctions_t &GetFunctions() const noexcept
			{
				return m_arFunctions;
			}

		protected:
			ILoconetSlot() { ; }

		protected:
			LoconetSlotFunctions_t m_arFunctions;

			dcclite::broker::DccAddress m_tLocomotiveAddress;

			//the id is useful for logging and debugging
			uint8_t m_uId = { 0 };			

			uint8_t m_uSpeed = { 0 };

			bool m_fForward = { true };
	};

	class LoconetService: public Service
	{	
		public:
			LoconetService(const std::string &name, Broker &broker, const rapidjson::Value& params, const Project& project);
		
			~LoconetService() override
			{
				//empty
			}
		

			static std::unique_ptr<Service> Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			const char *GetTypeName() const noexcept override
			{
				return "LoconetService";
			}
	};
}
