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
#include "TurnoutDecoder.h"

namespace dcclite::broker
{
	void NetworkTask::MarkFailed(std::string reason)
	{
		m_strMessage = std::move(reason);
		dcclite::Log::Error("{}", m_strMessage);

		m_fFailed = true;
		m_fFinished = true;

		this->NotifyObserver();
		m_rclOwner.TaskServices_ForgetTask(*this);
	}
}

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
				m_clThinker{ "DownloadEEPromTask::Thinker", THINKER_MF_LAMBDA(OnThink)}
			{
				//start running
				m_clThinker.Schedule({});
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
					m_clThinker.Schedule({});
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
					m_clThinker.Schedule(time + 10ms);
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

		m_clThinker.Schedule(time + DOWNLOAD_RETRY_TIMEOUT);		
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
					m_clThinker.Schedule(time + 100ms);

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
				{
					//WTF?					
					this->MarkFailed(fmt::format("[DownloadEEPromTask::Update] Invalid state {}", magic_enum::enum_name(m_kState)));
				}
				
		}		
	}

	//
	//
	// Servo Programmer Task
	//
	//	

	/**
	* ServoTurnoutProgrammerTask Packet format


	  0  1  2  3  4  5  6  7
	+--+--+--+--+--+--+--+--+		
	|          MSG          |
	+--+--+--+--+--+--+--+--+		
	|                       |
	//       MSG DATA       //
	|                       |
	+--+--+--+--+--+--+--+--+

	*/

	class ServoTurnoutProgrammerTask: public NetworkTaskImpl, public IServoProgrammerTask
	{
		public:
			ServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer, ServoTurnoutDecoder &decoder);

			void OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time) override;			

			void Abort() noexcept override;

			void Stop() noexcept override;

		private:			
			void SetPosition(const uint8_t position) override;	

			void DeployChanges(const uint8_t flags, const uint8_t startPos, const uint8_t endPos, std::chrono::milliseconds operationTime) override;			

			template<typename T>
			void GotoState();
			
			void GotoStateFailure(const char *stateName, const ServoProgammerClientErrors errorCode);
			void GotoStateFailure(std::string reason);

			void GotoRunningState();


			void FillPacket(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType, const ServoProgrammerServerMsgTypes msg);			
			
			struct State
			{				
				virtual void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) = 0;				

				virtual ~State() {};

				protected:
					State(ServoTurnoutProgrammerTask &self):
						m_rclSelf(self)
					{
						//empty
					}
					
					ServoTurnoutProgrammerTask &m_rclSelf;
			};

			struct NullState: State
			{
				explicit NullState(ServoTurnoutProgrammerTask &self):
					State{ self }
				{
					//empty
				}

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override
				{
					throw std::logic_error("[ServoTurnoutProgrammerTask::NullState::OnPacket] What?");
				}
			};

			struct StartingState: State
			{	
				explicit StartingState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override;

				private:
					void SendStartPacket(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;
			};

			struct RunningState: State
			{
				explicit RunningState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override;

				void OnNewServoPosition();

				private:
					void OnThink(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;

					uint32_t	m_u32Sequence = 0;
					uint32_t	m_u32AckSequence = 0;
			};

			struct TerminalState: State
			{
				protected:
					explicit TerminalState(ServoTurnoutProgrammerTask &self):
						State{ self }
					{
						//empty
					}
			};
			
			struct DeployState: TerminalState
			{
				explicit DeployState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override;

				private:
					void SendDeployPacket(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;
			};

			struct StoppingState: TerminalState
			{
				explicit StoppingState(ServoTurnoutProgrammerTask &self);

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override;

				private:
					void SendStopPacket(const dcclite::Clock::TimePoint_t time);

				private:
					Thinker	m_clThinker;
			};

			struct FailureState: TerminalState
			{
				explicit FailureState(ServoTurnoutProgrammerTask &self, std::string reason);

				void OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time) override;
			};

			struct DeployData
			{
				DeployData(const uint8_t flags, const uint8_t startPos, const uint8_t endPos, const std::chrono::milliseconds operationTime):
					m_fFlags{flags},
					m_u8StartPos{startPos > endPos ? endPos : startPos},
					m_u8EndPos{ startPos > endPos ? startPos : endPos},
					m_tOperationTime{ operationTime }
				{
					//empty
				}

				const uint8_t m_fFlags;
				const uint8_t m_u8StartPos;
				const uint8_t m_u8EndPos;
				const std::chrono::milliseconds m_tOperationTime;
			};

		private:
			ServoTurnoutDecoder &m_rclDecoder;						

			std::variant< 
				StartingState, 
				RunningState, 
				StoppingState, 
				FailureState, 
				DeployState,				
				NullState>				m_vState;

			std::optional<DeployData>	m_tDeployData;

			bool						m_fStopRequested = false;

			std::uint8_t		m_u8DecoderIndex;

			std::uint8_t		m_u8ServoPosition;
			std::uint8_t		m_u8ServoRemotePosition;			
	};

	ServoTurnoutProgrammerTask::ServoTurnoutProgrammerTask(INetworkDevice_TaskServices &owner, const uint32_t taskId, IObserver *observer, ServoTurnoutDecoder &decoder):
		NetworkTaskImpl{ owner, taskId, observer },
		m_rclDecoder{ decoder },
		m_u8DecoderIndex{ owner.FindDecoderIndex(decoder) },
		m_vState{ NullState{*this} }
	{		
		m_u8ServoPosition = decoder.GetStartPosition();

		//put a diferent number to force an update after programmer is connected
		m_u8ServoRemotePosition = m_u8ServoPosition + 1;

		//
		//After this is initialized, start the state machine
		m_vState.emplace<StartingState>( *this );
	}

	void ServoTurnoutProgrammerTask::OnPacket(dcclite::Packet &packet, const dcclite::Clock::TimePoint_t time)
	{		
		const auto msg = static_cast<ServoProgrammerClientMsgTypes>(packet.Read<uint8_t>());

		std::visit([&packet, msg, time](ServoTurnoutProgrammerTask::State &state) {state.OnPacket(packet, msg, time); }, m_vState);
	}


	/// <summary>
	/// This is expected to be called only when the connection from the NetworkDevice to the device is lost
	/// 
	/// So it does not cleanup things, just go to a terminal state (the task becomes a zombie) and the owners can later destroy it
	/// </summary>
	/// 
	void ServoTurnoutProgrammerTask::Abort() noexcept
	{			
		this->GotoStateFailure("[ServoTurnoutProgrammerTask::Abort] Abort was called");

		this->MarkAbort();				
	}

	void ServoTurnoutProgrammerTask::Stop() noexcept
	{
		//Finished? Ignore...
		if (this->HasFinished())
			return;

		//Already stopping? Ignore....
		if (std::holds_alternative<StoppingState>(m_vState))
			return;

		//Flag it, so later state transitions may intercept it
		m_fStopRequested = true;

		//
		//If on running state, it is safe to transition to stop mode
		if (std::holds_alternative<RunningState>(m_vState))
		{
			this->GotoState<StoppingState>();			
		}

		//
		//any other state: it is already stopping or entering failure mode, so we do not care
		//All other cases: if will be intercepted by "OnStateChange"
	}

	void ServoTurnoutProgrammerTask::DeployChanges(const uint8_t flags, const uint8_t startPos, const uint8_t endPos, std::chrono::milliseconds operationTime)
	{
		if (this->HasFinished())
			throw std::runtime_error("[ServoTurnoutProgrammerTask::SetPosition] Task has finished");

		ServoTurnoutDecoder::CheckServoData(startPos, endPos, "[ServoTurnoutProgrammerTask::DeployChanges]");

		m_tDeployData.emplace(flags, startPos, endPos, operationTime);

		//If on running state, Deploy the data
		if (auto state = std::get_if<RunningState>(&m_vState))
		{
			this->GotoState<DeployState>();
		}
		else
		{
			//any other state will finish the task or transit to the Running state and will later handle the Deploy
		}
	}

	void ServoTurnoutProgrammerTask::FillPacket(dcclite::Packet &packet, const uint32_t taskId, const NetworkTaskTypes taskType, const ServoProgrammerServerMsgTypes msg)
	{
		m_rclOwner.TaskServices_FillPacketHeader(packet, taskId, taskType);

		packet.Write8(static_cast<uint8_t>(msg));
	}	

	void ServoTurnoutProgrammerTask::SetPosition(const uint8_t position)
	{
		if (this->HasFinished())
			throw std::runtime_error("[ServoTurnoutProgrammerTask::SetPosition] Task has finished");

		//
		//Store it
		m_u8ServoPosition = position;

		//If on running state, notify it that new position is available
		if (auto state = std::get_if<RunningState>(&m_vState))
			state->OnNewServoPosition();		

		//
		//else... when running state loads up, it will grab the position and set it
	}	

	static ServoProgammerClientErrors ReadClientErrorValue(dcclite::Packet &packet)
	{
		return static_cast<ServoProgammerClientErrors>(packet.ReadByte());
	}

	static void ReportPacketError(dcclite::Packet &packet)
	{
		auto error = ReadClientErrorValue(packet);

		Log::Error("[ServoTurnoutProgrammerTask::StartingState::OnPacket] Task start failed: {}", magic_enum::enum_name(error));
	}

	//
	//
	// States Transitions
	//
	//

	template <typename T>
	void ServoTurnoutProgrammerTask::GotoState()
	{
		static_assert(!std::is_same<T, RunningState>::value, "Use GotoRunningState");

		m_vState.emplace<T>(*this);
	}

	void ServoTurnoutProgrammerTask::GotoRunningState()
	{
		m_vState.emplace<RunningState>(*this);

		if (m_fStopRequested)
		{
			this->GotoState<StoppingState>();

			return;
		}

		if (m_tDeployData)
		{
			this->GotoState<DeployState>();

			return;
		}

		//
		//If terminal already requested a new position, we may intercept it now and request the servo to move
		auto state = std::get_if<RunningState>(&m_vState);
		state->OnNewServoPosition();
	}

	void ServoTurnoutProgrammerTask::GotoStateFailure(const char *stateName, const ServoProgammerClientErrors errorCode)
	{
		this->GotoStateFailure(fmt::format(
			"[ServoTurnoutProgrammerTask::GotoStateFailure] Task {} on state {} received failure packet {}",
			m_u32TaskId,
			stateName,
			magic_enum::enum_name(errorCode)
		));
	}

	void ServoTurnoutProgrammerTask::GotoStateFailure(std::string reason)
	{
		m_vState.emplace<FailureState>(*this, std::move(reason));
	}

	//
	//
	// State Machine
	//
	//

	ServoTurnoutProgrammerTask::StartingState::StartingState(ServoTurnoutProgrammerTask &self):
		State{self},
		m_clThinker{ "ServoTurnoutProgrammerTask::Thinker", THINKER_MF_LAMBDA(SendStartPacket)}
	{		
		this->SendStartPacket(dcclite::Clock::DefaultClock_t::now());		
	}

	void ServoTurnoutProgrammerTask::StartingState::SendStartPacket(const dcclite::Clock::TimePoint_t time)
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::StartingState::SendStartPacket] Sent start packet - {}", m_rclSelf.m_u32TaskId);

		m_clThinker.Schedule(time + 50ms);

		//
		// Start it
		dcclite::Packet packet;

		m_rclSelf.FillPacket(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER, dcclite::ServoProgrammerServerMsgTypes::START);
		packet.Write8(m_rclSelf.m_u8DecoderIndex);

		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
	}

	void ServoTurnoutProgrammerTask::StartingState::OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time)
	{		
		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::FAILURE:
				dcclite::Log::Trace("[ServoTurnoutProgrammerTask::StartingState::OnPacket] Got failure packet");
				
				m_rclSelf.GotoStateFailure("StartingState", ReadClientErrorValue(packet));
				break;

			case ServoProgrammerClientMsgTypes::READY:
				dcclite::Log::Trace("[ServoTurnoutProgrammerTask::StartingState::OnPacket] Got READY packet");

				m_rclSelf.GotoRunningState();
				break;

			default:
				dcclite::Log::Error("[ServoTurnoutProgrammerTask::StartingState::OnPacket] Unexpected packet: {}", magic_enum::enum_name(msg));
				break;
		}
	}
	
	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::RunningState::RunningState(ServoTurnoutProgrammerTask &self):
		State{ self },
		m_clThinker{"ServoTurnoutProgrammerTask::RunningState", THINKER_MF_LAMBDA(OnThink)}
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::RunningState::RunningState] Running");
	}
	
	void ServoTurnoutProgrammerTask::RunningState::OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time)
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::RunningState::OnPacket] Got packet");
		
		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::SERVO_MOVED:
				{
					const auto ackSequence = packet.Read<uint32_t>();

					//old packet? if so, drop it
					if (ackSequence < m_u32AckSequence)
						return;
					
					m_u32AckSequence = ackSequence;

					m_rclSelf.m_u8ServoRemotePosition = packet.ReadByte();

					if (m_rclSelf.m_u8ServoPosition != m_rclSelf.m_u8ServoRemotePosition)
					{
						this->OnNewServoPosition();
					}
					else
					{
						//reached desired position, so no timeout
						m_clThinker.Cancel();
					}
				}
				break;	

			case ServoProgrammerClientMsgTypes::READY:
				//
				//ignore - we may have sent two starts and received the first one ack (and the second one still in the flight)
				// and later we got the second one, so ignore it
				//
				break;

			case ServoProgrammerClientMsgTypes::FAILURE:
				dcclite::Log::Error("[ServoTurnoutProgrammerTask::RunningState::OnPacket] Got failure packet");
				ReportPacketError(packet);				
				break;

			default:
				dcclite::Log::Error("[ServoTurnoutProgrammerTask::RunningState::OnPacket] Unexpected packet: {}", magic_enum::enum_name(msg));
				break;
		}
	}		

	void ServoTurnoutProgrammerTask::RunningState::OnNewServoPosition()
	{
		dcclite::Packet packet;

		m_rclSelf.FillPacket(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER, dcclite::ServoProgrammerServerMsgTypes::MOVE_SERVO);
		packet.Write32(++m_u32Sequence);
		packet.Write8(m_rclSelf.m_u8ServoPosition);

		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);

		m_clThinker.Schedule(dcclite::Clock::DefaultClock_t::now() + 50ms);
	}

	void ServoTurnoutProgrammerTask::RunningState::OnThink(const dcclite::Clock::TimePoint_t time)
	{
		//
		//servo set position may have timeout
		this->OnNewServoPosition();
	}

	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::DeployState::DeployState(ServoTurnoutProgrammerTask &self):
		TerminalState{ self },
		m_clThinker{ "ServoTurnoutProgrammerTask::DeployState", THINKER_MF_LAMBDA(SendDeployPacket)}
	{
		this->SendDeployPacket(dcclite::Clock::DefaultClock_t::now());
	}


	void ServoTurnoutProgrammerTask::DeployState::OnPacket(dcclite::Packet &dataPacket, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time)
	{
		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::SERVO_MOVED:
			case ServoProgrammerClientMsgTypes::READY:
				//
				// ignore those
				//
				break;

			case ServoProgrammerClientMsgTypes::FAILURE:
				//The deploy itself cannot fail, but it may have processed a later packet and task was already destroyed... so we ignore

				//Fallthrought

				//are we done?
			case ServoProgrammerClientMsgTypes::DEPLOY_FINISHED:
				dcclite::Log::Trace("[ServoTurnoutProgrammerTask::DeployState::OnPacket] Got DEPLOY_FINISHED packet {}", m_rclSelf.m_u32TaskId);

				{
					const auto &deployData = m_rclSelf.m_tDeployData.value();

					m_rclSelf.m_rclDecoder.UpdateData(deployData.m_fFlags, deployData.m_u8StartPos, deployData.m_u8EndPos, deployData.m_tOperationTime);
				}
				
				{
					dcclite::Packet packet;

					m_rclSelf.FillPacket(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER, dcclite::ServoProgrammerServerMsgTypes::DEPLOY_FINISHED_ACK);
					m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
				}																			

				m_clThinker.Cancel();
				m_rclSelf.MarkFinished();
				break;

			default:
				dcclite::Log::Error("[ServoTurnoutProgrammerTask::DeployState::OnPacket] Unexpected packet: {}", magic_enum::enum_name(msg));
				break;
		}
	}

	void ServoTurnoutProgrammerTask::DeployState::SendDeployPacket(const dcclite::Clock::TimePoint_t time)
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::DeployState::SendDeployPacket] Sending Deploy packet {} {}", m_rclSelf.m_u32TaskId, time.time_since_epoch().count());

		m_clThinker.Schedule(time + 50ms);

		//
		// Deploy it
		dcclite::Packet packet;

		const auto &deployData = m_rclSelf.m_tDeployData.value();

		m_rclSelf.FillPacket(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER, dcclite::ServoProgrammerServerMsgTypes::DEPLOY);

		packet.Write8(deployData.m_fFlags);
		packet.Write8(deployData.m_u8StartPos);
		packet.Write8(deployData.m_u8EndPos);
		packet.Write8(ServoTurnoutDecoder::TimeToTicks(deployData.m_tOperationTime, deployData.m_u8StartPos, deployData.m_u8EndPos));

		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
	}


	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::StoppingState::StoppingState(ServoTurnoutProgrammerTask &self):
		TerminalState{ self },
		m_clThinker{"ServoTurnoutProgrammerTask::StoppingState", THINKER_MF_LAMBDA(SendStopPacket)}
	{
		this->SendStopPacket(dcclite::Clock::DefaultClock_t::now());
	}

	void ServoTurnoutProgrammerTask::StoppingState::OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, const dcclite::Clock::TimePoint_t time)
	{		
		switch (msg)
		{
			case ServoProgrammerClientMsgTypes::SERVO_MOVED:
			case ServoProgrammerClientMsgTypes::READY:
				//
				// ignore those
				//
				break;

			case ServoProgrammerClientMsgTypes::FAILURE:								
				m_rclSelf.GotoStateFailure("StoppingState", ReadClientErrorValue(packet));				
				break;

			//are we done?
			case ServoProgrammerClientMsgTypes::FINISHED:
				dcclite::Log::Trace("[ServoTurnoutProgrammerTask::StoppingState::OnPacket] Got FINISHED packet {}", m_rclSelf.m_u32TaskId);				

				m_clThinker.Cancel();
				m_rclSelf.MarkFinished();								
				break;		

			default:
				dcclite::Log::Error("[ServoTurnoutProgrammerTask::StoppingState::OnPacket] Unexpected packet: {}", magic_enum::enum_name(msg));
				break;
		}
	}

	void ServoTurnoutProgrammerTask::StoppingState::SendStopPacket(const dcclite::Clock::TimePoint_t time)
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::StoppingState::SendStopPacket] Sending stop packet {} {}", m_rclSelf.m_u32TaskId, time.time_since_epoch().count());

		m_clThinker.Schedule(time + 50ms);

		//
		// Stop it
		dcclite::Packet packet;

		m_rclSelf.FillPacket(packet, m_rclSelf.m_u32TaskId, NetworkTaskTypes::TASK_SERVO_PROGRAMMER, dcclite::ServoProgrammerServerMsgTypes::STOP);
		
		m_rclSelf.m_rclOwner.TaskServices_SendPacket(packet);
	}


	//
	//
	//
	//
	//

	ServoTurnoutProgrammerTask::FailureState::FailureState(ServoTurnoutProgrammerTask &self, std::string reason):
		TerminalState{ self }
	{
		dcclite::Log::Trace("[ServoTurnoutProgrammerTask::FailureState::FailureState] Entered failure state {}", m_rclSelf.m_u32TaskId);

		self.MarkFailed(std::move(reason));
	}

	void ServoTurnoutProgrammerTask::FailureState::OnPacket(dcclite::Packet &packet, const ServoProgrammerClientMsgTypes msg, dcclite::Clock::TimePoint_t time)
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
