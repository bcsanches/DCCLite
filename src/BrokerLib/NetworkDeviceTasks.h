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

#include <functional>
#include <string>
#include <variant>

#include "IDccLiteService.h"
#include "Device.h"
#include "IDevice.h"
#include "Packet.h"
#include "PinManager.h"
#include "Socket.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{ 
	class ServoTurnoutDecoder;

	class NetworkTask
	{
		public:
			//
			// When the task finishes, success or not, this must return true			
			//
			virtual bool HasFinished() const noexcept = 0;

			//
			// When the task fails to complete, this must return true (results must be ignored / discarded)
			//
			virtual bool HasFailed() const noexcept = 0;

	};	

	typedef std::vector<uint8_t> DownloadEEPromTaskResult_t;

	namespace detail
	{
		class INetworkDevice_TaskServices
		{
			public:
				virtual void TaskServices_FillPacketHeader(dcclite::Packet &packet) const noexcept = 0;

				virtual void TaskServices_SendPacket(dcclite::Packet &packet) = 0;

				[[nodiscard]] virtual bool IsConnectionStable() const noexcept = 0;
		};

		/**
		* TASK_DATA Packet format

		/**
		* Packet format

										1  1  1  1  1  1
		  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
		+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		|                    TASKID                     |
		|												| 32 bits (uint32) - consumed by network device for finding the task
		+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
		|                                               |
		//                  TASK DATA                   //
		|                                               |
		+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

		*/

		class NetworkTaskImpl: public NetworkTask
		{
			protected:
				NetworkTaskImpl(INetworkDevice_TaskServices &owner, const uint32_t taskId):
					m_u32TaskId{ taskId },
					m_rOwner{ owner }
				{
					//empty
				}

			public:
				virtual void Abort() noexcept = 0;

				//
				//
				//
				virtual void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) = 0;

				//
				// Do some work, returns true if still has pending work. 
				//			
				[[nodiscard]] virtual bool Update(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time) noexcept = 0;

				[[nodiscard]] inline uint32_t GetTaskId() const noexcept
				{
					return m_u32TaskId;
				}

			protected:
				const uint32_t m_u32TaskId;

				INetworkDevice_TaskServices &m_rOwner;
		};		

		extern std::shared_ptr<NetworkTaskImpl> StartDownloadEEPromTask(INetworkDevice_TaskServices &device, const uint32_t taskId, DownloadEEPromTaskResult_t &resultsStorage);
		extern std::shared_ptr<NetworkTaskImpl> StartServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, ServoTurnoutDecoder &decoder);
	}	
}
