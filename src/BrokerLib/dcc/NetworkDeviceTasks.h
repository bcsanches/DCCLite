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

#include <memory>
#include <string>

#include <stdint.h>
#include <vector>

#include <dcclite/Clock.h>
#include <dcclite/RName.h>

#include <dcclite_shared/Packet.h>

namespace dcclite::broker
{ 
	class NetworkTask;
	class Decoder;

	namespace detail
	{
		class INetworkDevice_TaskServices
		{
			public:
				virtual void TaskServices_FillPacketHeader(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType) const noexcept = 0;

				virtual void TaskServices_SendPacket(dcclite::Packet &packet) = 0;

				[[nodiscard]] virtual bool IsConnectionStable() const noexcept = 0;

				virtual void TaskServices_ForgetTask(NetworkTask &task) = 0;

				[[nodiscard]] virtual uint8_t FindDecoderIndex(const Decoder &decoder) const = 0;

				virtual void TaskServices_Disconnect() = 0;
		};
	}

	class ServoTurnoutDecoder;	

	class NetworkTask
	{
		public:
			virtual ~NetworkTask() = default;

			class IObserver
			{
				public:
					virtual void OnNetworkTaskStateChanged(NetworkTask &task) = 0;
			};			

			[[nodiscard]] inline uint32_t GetTaskId() const noexcept
			{
				return m_u32TaskId;
			}

			//
			// When the task finishes, success or not, this must return true			
			//			
			[[nodiscard]] inline bool HasFinished() const noexcept
			{
				return m_fFinished;
			}			

			//
			// When the task fails to complete, this must return true (results must be ignored / discarded)
			//			
			[[nodiscard]] inline bool HasFailed() const noexcept
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

			inline const std::string &GetMessage() const noexcept
			{
				return m_strMessage;
			}

		protected:
			inline NetworkTask(detail::INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer = nullptr) noexcept:
				m_rclOwner{ owner },				
				m_pclObserver{ observer },
				m_u32TaskId{ taskId }
			{
				//empty
			}

			inline void NotifyObserver()
			{
				if (m_pclObserver)
					m_pclObserver->OnNetworkTaskStateChanged(*this);
			}

			//
			// The task may be deleted after this method returns
			//
			inline void MarkFinished()
			{
				m_fFinished = true;

				this->NotifyObserver();	
				m_rclOwner.TaskServices_ForgetTask(*this);
			}

			//
			// The task may be deleted after this method returns
			//
			void MarkFailed(std::string reason);

			//
			// The task may be deleted after this method returns
			//
			inline void MarkAbort()
			{
				m_fFailed = true;
				m_fFinished = true;

				this->NotifyObserver();

				//
				// Abort does not need to call forget, the network device will do it...
				//m_rclOwner.TaskServices_ForgetTask(*this);
			}


		protected:
			detail::INetworkDevice_TaskServices	&m_rclOwner;

		private:
			std::string		m_strMessage;
			IObserver		*m_pclObserver;

		protected:
			const uint32_t						m_u32TaskId;

		private:
			bool m_fFinished = false;
			bool m_fFailed = false;			
	};	

	class IServoProgrammerTask
	{
		public:
			virtual void DeployChanges(const uint8_t flags, const uint8_t startPos, const uint8_t endPos, std::chrono::milliseconds operationTime) = 0;

			virtual void SetPosition(const uint8_t position) = 0;
	};

	typedef std::vector<uint8_t> DownloadEEPromTaskResult_t;

	namespace detail
	{

		/**
		* TASK_DATA Packet format
		*
		*
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
					NetworkTask{owner, taskId, observer}
				{
					//empty
				}

			public:

				/**
				* Abort means that the connection was lost, so the task should stop whatever it is doing and should not try to communicate with the remote device anymore
				* 
				* Also, the NetworkDevice will automatically forget the task after calling abort, so task may be destroyed after it	
				*
				*/
				virtual void Abort() noexcept = 0;				

				//
				//
				//
				virtual void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) = 0;							
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

		extern std::shared_ptr<NetworkTaskImpl> StartDeviceRenameTask(
			INetworkDevice_TaskServices &owner,
			const uint32_t taskId,
			NetworkTask::IObserver *observer,
			RName newName
		);

		extern std::shared_ptr<NetworkTaskImpl> StartDeviceClearEEPromTask(
			INetworkDevice_TaskServices &owner, 
			const uint32_t taskId, 
			NetworkTask::IObserver *observer
		);
	}	
}
