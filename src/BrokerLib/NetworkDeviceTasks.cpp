// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "NetworkDevice.h"

#include <magic_enum.hpp>

#include "BitPack.h"
#include "IDccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "Project.h"
#include "SensorDecoder.h"

namespace dcclite::broker::detail
{		
	//
	//
	// TASKS
	//
	//
	//	

	/**
	* DownloadEEPromTask Packet format

								
	  0  1  2  3  4  5  6  7  
	+--+--+--+--+--+--+--+--+
	|       SEQUENCE        |
	+--+--+--+--+--+--+--+--+
	|      NUM SLICES       |
	+--+--+--+--+--+--+--+--+
	|      SLICE SIZE       |
	+--+--+--+--+--+--+--+--+
	|                       |
	//        SLICE         //
	|                       |
	+--+--+--+--+--+--+--+--+

	*/

	using namespace std::chrono_literals;

	static auto constexpr DOWNLOAD_RETRY_TIMEOUT = 100ms;


	class DownloadEEPromTask: public NetworkTaskImpl
	{
		public:
			DownloadEEPromTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, DownloadEEPromTaskResult_t &results):
				NetworkTaskImpl{owner, taskId},
				m_vecResults{ results }
			{
				//empty
			}

			bool HasFinished() const noexcept override
			{
				return m_fFinished;
			}

			bool HasFailed() const noexcept override
			{
				return m_fFailed;
			}

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;

			bool Update(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time) noexcept override;

			void Abort() noexcept override;
			void Stop() noexcept override;

		private:
			void ReadSlice(dcclite::Packet &packet, const uint8_t sliceSize, const uint8_t sequence);

			void RequestSlice(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time, const uint8_t sliceNum);

		private:
			enum class States
			{
				WAITING_CONNECTION,
				START_DOWNLOAD,
				DOWNLOADING				
			};

			friend class NetworkDevice;
			
			DownloadEEPromTaskResult_t	&m_vecResults;		
			std::vector<bool>			m_vecSlicesAck;			

			States	m_kState = States::WAITING_CONNECTION;		

			dcclite::Clock::TimePoint_t m_tWaitTimeout;
			
			bool m_fFinished = false;
			bool m_fFailed = false;
	};

	void DownloadEEPromTask::Abort() noexcept
	{		
		m_fFailed = true;
		m_fFinished = true;
	}	

	void DownloadEEPromTask::Stop() noexcept
	{
		if(!m_fFinished)
			this->Abort();
	}

	void DownloadEEPromTask::ReadSlice(dcclite::Packet &packet, const uint8_t sliceSize, const uint8_t sequence)
	{
		for (auto i = 0; i < sliceSize; ++i)
		{
			m_vecResults[(sliceSize * sequence) + i] = packet.ReadByte();
		}
		m_vecSlicesAck[sequence] = true;		

		Log::Info("[DownloadEEPromTask::Update] Received {} bytes for slice {}", sliceSize, sequence);
	}

	void DownloadEEPromTask::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
		switch (m_kState)
		{
			//We should not get messages on this state...
			case States::WAITING_CONNECTION:
				return;

			case States::START_DOWNLOAD:
				{
					auto sequence = packet.ReadByte();
					auto numSlices = packet.ReadByte();
					auto sliceSize = packet.ReadByte();

					if (sequence >= numSlices)
					{
						//corrupted data?
						dcclite::Log::Error("[DownloadEEPromTask::OnPacket] sequence({}) >= numSlices({})", sequence, numSlices);

						return;
					}					

					m_vecResults.resize(numSlices * sliceSize);
					m_vecSlicesAck.resize(numSlices);

					Log::Info("[DownloadEEPromTask::Update] Downloading {} bytes / {} slices", m_vecResults.size(), numSlices);

					this->ReadSlice(packet, sliceSize, sequence);

					m_kState = States::DOWNLOADING;

					//force restart
					m_tWaitTimeout = time;
				}
				break;

			case States::DOWNLOADING:
				{
					auto sequence = packet.ReadByte();
					auto numSlices = packet.ReadByte();
					auto sliceSize = packet.ReadByte();

					if (sequence >= numSlices)
					{
						//corrupted data?
						dcclite::Log::Error("[DownloadEEPromTask::OnPacket] sequence({}) >= numSlices({})", sequence, numSlices);

						return;
					}

					//already got this packet?
					if (m_vecSlicesAck[sequence])
						return;

					this->ReadSlice(packet, sliceSize, sequence);					

					//force restart
					m_tWaitTimeout = time;
				}
				break;
		}
	}

	void DownloadEEPromTask::RequestSlice(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time, const uint8_t sliceNum)
	{
		dcclite::Packet packet;		

		owner.TaskServices_FillPacketHeader(packet);		

		packet.Write8(static_cast<uint8_t>(NetworkTaskTypes::TASK_DOWNLOAD_EEPROM));
		packet.Write32(m_u32TaskId);

		//slice number
		packet.Write8(static_cast<uint8_t>(sliceNum));

		owner.TaskServices_SendPacket(packet);		

		m_tWaitTimeout = time + DOWNLOAD_RETRY_TIMEOUT;
	}

	bool DownloadEEPromTask::Update(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time) noexcept
	{		
		assert(!m_fFailed);
		assert(!m_fFinished);
		
		switch (m_kState)
		{
			case States::WAITING_CONNECTION:				
				//Wait for a stable connection (after config, sync, etc)
				if (!m_rOwner.IsConnectionStable())
					return true;

				m_kState = States::START_DOWNLOAD;		
				Log::Info("[DownloadEEPromTask::Update]: Requesting data {} bytes", m_vecResults.size());

				[[fallthrough]];
			case States::START_DOWNLOAD:

				//too soon?
				if (m_tWaitTimeout <= time)
				{
					//slice number, request 0... always valid
					this->RequestSlice(owner, time, 0);
				}				
				return true;				

			case States::DOWNLOADING:
				//too soon?
				if (m_tWaitTimeout > time)
					return true;

				{
					assert(m_vecSlicesAck.size() < 256);

					int packetCount = 0;

					//check for missing slices
					for (size_t i = 0, sz = m_vecSlicesAck.size(); i < sz; ++i)
					{
						if (m_vecSlicesAck[i])
							continue;

						++packetCount;
						this->RequestSlice(owner, time, static_cast<uint8_t>(i));						
					}

					if (packetCount)
						return true;

					//
					// received all packets...
					m_fFinished = true;

					Log::Info("[DownloadEEPromTask::Update]: finished download of {} bytes", m_vecResults.size());

					return false;
				}
				break;

			default:
				//WTF?
				Log::Error("[DownloadEEPromTask::Update] Invalid state {}", magic_enum::enum_name(m_kState));

				m_fFailed = true;
				return false;
				break;
		}		
	}

	//
	//
	// Servo Programmer Task
	//
	//	

	class ServoTurnoutProgrammerTask: public NetworkTaskImpl, public IServoProgrammerTask
	{
		public:
			inline ServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, ServoTurnoutDecoder &decoder):
				NetworkTaskImpl{ owner, taskId },
				m_rclDecoder{decoder}
			{
				//empty
			}			

			bool HasFinished() const noexcept override
			{
				return m_fFinished;
			}

			bool HasFailed() const noexcept override
			{
				return m_fFailed;
			}

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;

			bool Update(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time) noexcept override;

			void Abort() noexcept override;

			void Stop() noexcept override;

		private:
			void SetStartPos(const uint8_t startPos) override;
			void SetEndPos(const uint8_t startPos) override;

			void SetInverted(const bool inverted) override;

		private:
			ServoTurnoutDecoder &m_rclDecoder;
			
			bool m_fFinished = false;
			bool m_fFailed = false;
	};

	void ServoTurnoutProgrammerTask::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{

	}

	bool ServoTurnoutProgrammerTask::Update(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time) noexcept
	{
		return true;
	}

	void ServoTurnoutProgrammerTask::Abort() noexcept
	{			
		m_fFailed = true;
		m_fFinished = true;
	}

	void ServoTurnoutProgrammerTask::Stop() noexcept
	{
		m_fFinished = true;
	}

	void ServoTurnoutProgrammerTask::SetStartPos(const uint8_t startPos)
	{

	}

	void ServoTurnoutProgrammerTask::SetEndPos(const uint8_t startPos)
	{

	}

	void ServoTurnoutProgrammerTask::SetInverted(const bool inverted)
	{

	}

	//
	//
	// Helpers
	//
	//	
	std::shared_ptr<NetworkTaskImpl> StartDownloadEEPromTask(INetworkDevice_TaskServices &device, const uint32_t taskId, DownloadEEPromTaskResult_t &resultsStorage)
	{		
		return std::make_shared<DownloadEEPromTask>(device, taskId, resultsStorage);
	}

	std::shared_ptr<NetworkTaskImpl> StartServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, ServoTurnoutDecoder &decoder)
	{
		return std::make_shared<ServoTurnoutProgrammerTask>(owner, taskId, decoder);
	}
}
