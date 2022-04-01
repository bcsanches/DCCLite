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
			class IObserver
			{
				public:
					virtual void OnNetworkTaskStateChanged(const NetworkTask &task) = 0;
			};


			inline NetworkTask(const uint32_t taskId, IObserver *observer = nullptr) noexcept:
				m_u32TaskId{ taskId },
				m_pclObserver{observer}
			{
				//empty
			}

			[[nodiscard]] inline uint32_t GetTaskId() const noexcept
			{
				return m_u32TaskId;
			}

			//
			// When the task finishes, success or not, this must return true			
			//			
			inline [[nodiscard]] bool HasFinished() const noexcept
			{
				return m_fFinished;
			}			

			//
			// When the task fails to complete, this must return true (results must be ignored / discarded)
			//			
			inline [[nodiscard]] bool HasFailed() const noexcept
			{
				return m_fFailed;
			}

			//
			// Ask the task to stop. It may continue futher processing for a graceful stop (that can later fail)
			//
			virtual void Stop() noexcept = 0;

			inline void SetObserver(IObserver *observer) noexcept
			{
				m_pclObserver = observer;
			}

		protected:
			inline void NotifyObserver()
			{
				if (m_pclObserver)
					m_pclObserver->OnNetworkTaskStateChanged(*this);
			}

			inline void MarkFinished()
			{
				m_fFinished = true;

				this->NotifyObserver();
			}

			inline void MarkFailed()
			{
				m_fFailed = true;

				this->NotifyObserver();
			}

			inline void MarkAbort()
			{
				m_fFailed = true;
				m_fFinished = true;

				this->NotifyObserver();
			}


		protected:
			const uint32_t	m_u32TaskId;

		private:
			IObserver		*m_pclObserver;

			bool m_fFinished = false;
			bool m_fFailed = false;

	};	

	class IServoProgrammerTask
	{
		public:
			virtual void SetStartPos(const uint8_t startPos) = 0;
			virtual void SetEndPos(const uint8_t startPos) = 0;

			virtual void SetInverted(const bool inverted) = 0;
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
				NetworkTaskImpl(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer):
					NetworkTask{taskId, observer},
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

			protected:				
				INetworkDevice_TaskServices &m_rOwner;
		};				

		extern std::shared_ptr<NetworkTaskImpl> StartDownloadEEPromTask(
			INetworkDevice_TaskServices	&device, 
			const uint32_t				taskId, 
			NetworkTask::IObserver		*observer,
			DownloadEEPromTaskResult_t	&resultsStorage
		);

		extern std::shared_ptr<NetworkTaskImpl> StartServoTurnoutProgrammerTask(
			INetworkDevice_TaskServices	&owner, 
			const uint32_t				taskId, 
			NetworkTask::IObserver		*observer,
			ServoTurnoutDecoder			&decoder
		);
	}	
}
