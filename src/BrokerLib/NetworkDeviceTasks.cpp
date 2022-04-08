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
#include "Thinker.h"

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
			DownloadEEPromTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer, DownloadEEPromTaskResult_t &results):
				NetworkTaskImpl{owner, taskId, observer },
				m_vecResults{ results },
				m_clThinker{ THINKER_MF_LAMBDA(OnThink) }
			{
				//start running
				m_clThinker.SetNext({});
			}			

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;			

			void Abort() noexcept override;
			void Stop() noexcept override;

		private:
			void ReadSlice(dcclite::Packet &packet, const uint8_t sliceSize, const uint8_t sequence);

			void RequestSlice(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time, const uint8_t sliceNum);

			void OnThink(const dcclite::Clock::TimePoint_t time);

		private:
			enum class States
			{
				WAITING_CONNECTION,
				START_DOWNLOAD,
				DOWNLOADING				
			};

			friend class NetworkDevice;

			Thinker						m_clThinker;
			
			DownloadEEPromTaskResult_t	&m_vecResults;		
			std::vector<bool>			m_vecSlicesAck;			

			States	m_kState = States::WAITING_CONNECTION;					
	};

	void DownloadEEPromTask::Abort() noexcept
	{		
		this->MarkAbort();		
	}	

	void DownloadEEPromTask::Stop() noexcept
	{
		if(!this->HasFinished())
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
					m_clThinker.SetNext({});
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

					//force restart ... but wait a bit, so more packets may arrive
					m_clThinker.SetNext(time + 10ms);
				}
				break;
		}
	}

	void DownloadEEPromTask::RequestSlice(INetworkDevice_TaskServices &owner, const dcclite::Clock::TimePoint_t time, const uint8_t sliceNum)
	{
		Log::Trace("[DownloadEEPromTask::Update]: requesting slice {}", (int)sliceNum);

		dcclite::Packet packet;		

		owner.TaskServices_FillPacketHeader(packet, m_u32TaskId, NetworkTaskTypes::TASK_DOWNLOAD_EEPROM);

		//slice number
		packet.Write8(static_cast<uint8_t>(sliceNum));

		owner.TaskServices_SendPacket(packet);		

		m_clThinker.SetNext(time + DOWNLOAD_RETRY_TIMEOUT);		
	}

	void DownloadEEPromTask::OnThink(const dcclite::Clock::TimePoint_t time)
	{		
		assert(!this->HasFailed());
		assert(!this->HasFinished());
		
		switch (m_kState)
		{
			case States::WAITING_CONNECTION:				
				//Wait for a stable connection (after config, sync, etc)
				if (!m_rclOwner.IsConnectionStable())
				{
					m_clThinker.SetNext(time + 100ms);

					return;
				}

				m_kState = States::START_DOWNLOAD;		
				Log::Info("[DownloadEEPromTask::Update]: Requesting data {} bytes", m_vecResults.size());

				[[fallthrough]];
			case States::START_DOWNLOAD:								
				//slice number, request 0... always valid
				this->RequestSlice(m_rclOwner, time, 0);
				break;

			case States::DOWNLOADING:

				{
					assert(m_vecSlicesAck.size() < 256);

					int packetCount = 0;

					//check for missing slices
					for (size_t i = 0, sz = m_vecSlicesAck.size(); i < sz; ++i)
					{
						if (m_vecSlicesAck[i])
							continue;

						++packetCount;
						this->RequestSlice(m_rclOwner, time, static_cast<uint8_t>(i));
					}

					//Do we still have work to do?
					if (packetCount)
						return ;
				}

				Log::Info("[DownloadEEPromTask::Update]: finished download of {} bytes", m_vecResults.size());

				//
				// received all packets...
				this->MarkFinished();				
				break;

			default:
				//WTF?
				Log::Error("[DownloadEEPromTask::Update] Invalid state {}", magic_enum::enum_name(m_kState));

				this->MarkFailed();				
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
			ServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer, ServoTurnoutDecoder &decoder);

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;			

			void Abort() noexcept override;

			void Stop() noexcept override;

		private:
			void SetStartPos(const uint8_t startPos) override;
			void SetEndPos(const uint8_t startPos) override;

			void SetInverted(const bool inverted) override;

			void SetPosition(const uint8_t position) override;	

			void GotoFailureState();
			void GotoRunningState();
			void GotoStoppingState();
			
			struct State
			{				
				virtual void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) = 0;				

				virtual ~State() {};

				protected:
					State(ServoTurnoutProgrammerTask &self):
						m_rclSelf(self)
					{
						//empty
					}
					
					ServoTurnoutProgrammerTask &m_rclSelf;
			};

			struct StartingState: State
			{	
				StartingState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;				

				private:
					void SendStartPacket(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;
			};

			struct RunningState: State
			{
				RunningState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;				
			};

			struct StoppingState: State
			{
				StoppingState(ServoTurnoutProgrammerTask &self);							

				void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;				

				private:
					void SendStopPacket(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;
			};

			struct FailureState: State
			{
				FailureState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;			
			};

		private:
			ServoTurnoutDecoder &m_rclDecoder;			

			std::variant< StartingState, RunningState, StoppingState, FailureState> m_vState;
	};

	ServoTurnoutProgrammerTask::ServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer, ServoTurnoutDecoder &decoder):
		NetworkTaskImpl{ owner, taskId, observer },
		m_rclDecoder{ decoder },		
		m_vState{ StartingState {*this} }
	{
		//empty
	}

	void ServoTurnoutProgrammerTask::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
		std::visit([&packet, time](ServoTurnoutProgrammerTask::State &state) {state.OnPacket(packet, time); }, m_vState);
	}

	void ServoTurnoutProgrammerTask::Abort() noexcept
	{			
		this->GotoFailureState();

		this->MarkAbort();				
	}

	void ServoTurnoutProgrammerTask::Stop() noexcept
	{
		if (this->HasFinished())
			return;

		if (std::holds_alternative<StoppingState>(m_vState))
			return;

		this->GotoStoppingState();		
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

	void ServoTurnoutProgrammerTask::SetPosition(const uint8_t position)
	{

	}	

	static void ReportPacketError(dcclite::Packet &packet)
	{
		auto error = static_cast<ServoProgammerClientErrors>(packet.ReadByte());

		Log::Error("[ServoTurnoutProgrammerTask::StartingState::OnPacket] Task start failed: {}", magic_enum::enum_name(error));
	}

	//
	//
	// States Transitions
	//
	//

	void ServoTurnoutProgrammerTask::GotoFailureState()
	{
		m_vState.emplace<FailureState>(*this);
	}

	void ServoTurnoutProgrammerTask::GotoRunningState()
	{
		m_vState.emplace<RunningState>(*this);
	}

	void ServoTurnoutProgrammerTask::GotoStoppingState()
	{
		m_vState.emplace<StoppingState>(*this);
	}

	//
	//
	// State Machine
	//
	//

	ServoTurnoutProgrammerTask::StartingState::StartingState(ServoTurnoutProgrammerTask &self):
		State{self},
		m_clThinker{ THINKER_MF_LAMBDA(SendStartPacket) }
	{		
		this->SendStartPacket(dcclite::Clock::DefaultClock_t::now());
	}

	void ServoTurnoutProgrammerTask::StartingState::SendStartPacket(const dcclite::Clock::TimePoint_t time)
	{
		m_clThinker.SetNext(time + 50ms);

		//
		// Start it
		dcclite::Packet packet;

		m_rclSelf.m_rclOwner.TaskServices_FillPacketHeader(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER);

		packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerServerMsgTypes::START));

		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
	}

	void ServoTurnoutProgrammerTask::StartingState::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
		auto msg = static_cast<dcclite::ServoProgrammerClientMsgTypes>(packet.ReadByte());

		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::FAILURE:
				ReportPacketError(packet);
				m_rclSelf.GotoFailureState();
				break;

			case ServoProgrammerClientMsgTypes::READY:
				m_rclSelf.GotoRunningState();
				break;
		}
	}
	
	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::RunningState::RunningState(ServoTurnoutProgrammerTask &self):
		State{ self }		
	{
		//empty
	}
	
	void ServoTurnoutProgrammerTask::RunningState::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
	}		

	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::StoppingState::StoppingState(ServoTurnoutProgrammerTask &self):
		State{ self },
		m_clThinker{THINKER_MF_LAMBDA(SendStopPacket)}
	{
		this->SendStopPacket(dcclite::Clock::DefaultClock_t::now());
	}

	void ServoTurnoutProgrammerTask::StoppingState::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
		auto msg = static_cast<dcclite::ServoProgrammerClientMsgTypes>(packet.ReadByte());

		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::FAILURE:
				ReportPacketError(packet);
				m_rclSelf.GotoFailureState();
				break;

			//are we done?
			case ServoProgrammerClientMsgTypes::FINISHED:
				m_clThinker.Cancel();
				m_rclSelf.MarkFinished();								
				break;
		}
	}

	void ServoTurnoutProgrammerTask::StoppingState::SendStopPacket(const dcclite::Clock::TimePoint_t time)
	{
		m_clThinker.SetNext(time + 50ms);

		//
		// Stop it
		dcclite::Packet packet;

		m_rclSelf.m_rclOwner.TaskServices_FillPacketHeader(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER);

		packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerServerMsgTypes::STOP));

		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
	}

	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::FailureState::FailureState(ServoTurnoutProgrammerTask &self):
		State{ self }
	{
		self.MarkFailed();
	}

	void ServoTurnoutProgrammerTask::FailureState::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{
		//ignore packets...
	}

	//
	//
	// Helpers
	//
	//	
	std::shared_ptr<NetworkTaskImpl> StartDownloadEEPromTask(INetworkDevice_TaskServices &device, const uint32_t taskId, NetworkTask::IObserver *observer, DownloadEEPromTaskResult_t &resultsStorage)
	{		
		return std::make_shared<DownloadEEPromTask>(device, taskId, observer, resultsStorage);
	}

	std::shared_ptr<NetworkTaskImpl> StartServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, NetworkTask::IObserver *observer, ServoTurnoutDecoder &decoder)
	{
		return std::make_shared<ServoTurnoutProgrammerTask>(owner, taskId, observer, decoder);
	}
}
