// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "ThrottleService.h"

#include <Log.h>

#include <fmt/chrono.h>
#include <variant>

#include "Clock.h"
#include "FmtUtils.h"
#include "LoconetService.h"
#include "NetMessenger.h"
#include "Parser.h"

///////////////////////////////////////////////////////////////////////////////
//
// Throttle
//
///////////////////////////////////////////////////////////////////////////////


class Throttle: public dcclite::IObject, public dcclite::broker::IThrottle
{
	public:
#if 1
		Throttle(const dcclite::NetworkAddress &serverAddress, const dcclite::broker::ILoconetSlot &owner) :
			IObject(std::move(fmt::format("slot[{}][{}]", owner.GetId(), owner.GetLocomotiveAddress().GetAddress()))),
			m_clServerAddress{serverAddress},
			m_vState{ ConnectState {serverAddress} },
			m_tLocomotiveAddress{ owner.GetLocomotiveAddress() },
			m_rclOwnerSlot{ owner }
		{
			m_pclCurrentState = std::get_if<ConnectState>(&m_vState);

			assert(m_pclCurrentState);
		}
#endif

		const char *GetTypeName() const noexcept override
		{
			return "Throttle";
		}

		void Update(const dcclite::Clock &clock)
		{
			m_pclCurrentState->Update(*this, clock.Ticks());
		}

		void OnSpeedChange() override
		{			
			if (m_pclConnectedState)
				m_pclConnectedState->SetSpeed(m_rclOwnerSlot.GetSpeed());
		}

		void OnForwardChange() override
		{	
			if (m_pclConnectedState)
				m_pclConnectedState->SetForward(m_rclOwnerSlot.IsForwardDir());
		}

		void OnFunctionChange(const uint8_t begin, const uint8_t end) override
		{
			if (!m_pclConnectedState)
				return;

			m_pclConnectedState->OnFunctionChange(begin, end, m_rclOwnerSlot.GetFunctions());
		}

		void OnEmergencyStop() override
		{
			if (!m_pclConnectedState)
				return;

			m_pclConnectedState->OnEmergencyStop();
		}

	private:
		void GotoConnectState()
		{
			m_vState = ConnectState{ m_clServerAddress };
			m_pclCurrentState = std::get_if<ConnectState>(&m_vState);

			assert(m_pclCurrentState);

			m_pclConnectedState = nullptr;
		}

		void GotoErrorState(std::string reason)
		{
			m_vState = ErrorState{std::move(reason)};
			m_pclCurrentState = std::get_if<ErrorState>(&m_vState);

			assert(m_pclCurrentState);

			m_pclConnectedState = nullptr;
		}

		void GotoHandShakeState()
		{
			auto connectingState = std::get_if<ConnectState>(&m_vState);

			if (!connectingState)
				throw std::logic_error("[Throttle::GotoHandShakeState] Invalid state, must be in ConnectingState state");

			m_vState = HandShakeState{ std::move(connectingState->m_clSocket) };
			m_pclCurrentState = std::get_if<HandShakeState>(&m_vState);

			assert(m_pclCurrentState);

			m_pclConnectedState = nullptr;
		}

		void GotoConfiguringThrottleIdState(const char *separator, const char *initialBuffer = "")
		{
			auto handShakeState = std::get_if<HandShakeState>(&m_vState);

			if (!handShakeState)
				throw std::logic_error("[Throttle::GotoConfiguringThrottleIdState] Invalid state, must be in handShakeState state");
			
			dcclite::Log::Debug("[Throttle::GotoConfiguringState] Detected line ending as {} - entering configuring state", separator[0] == '\n' ? "\\n" : separator[1] ? "\\r\\n" : "\\r");
			
			m_vState.emplace<ConfiguringThrottleIdState>( *this, dcclite::NetMessenger{std::move(handShakeState->m_clSocket), separator, initialBuffer});
			m_pclCurrentState = std::get_if<ConfiguringThrottleIdState>(&m_vState);

			assert(m_pclCurrentState);

			m_pclConnectedState = nullptr;
		}

		void GotoConnectedState()
		{
			auto configuringState = std::get_if<ConfiguringThrottleIdState>(&m_vState);

			if (!configuringState)
				throw std::logic_error("[Throttle::GotoConnectedState] Invalid state, must be in ConfiguringState state");

			auto tmp = std::move(*configuringState);
			m_vState.emplace<ConnectedState>(*this, std::move(tmp));
			m_pclCurrentState = m_pclConnectedState = std::get_if<ConnectedState>(&m_vState);	

			assert(m_pclCurrentState);
		}

	private:		
		struct State
		{
			virtual void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) = 0;
		};

		struct ErrorState : State
		{
			ErrorState(std::string reason)
			{
				dcclite::Log::Error(reason.c_str());
			}

			void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
			{
				//empty
			}
		};

		struct ConnectState: State
		{							
			public:
				ConnectState(const dcclite::NetworkAddress &serverAddress)
				{
					if (!m_clSocket.StartConnection(0, dcclite::Socket::Type::STREAM, serverAddress))
						throw std::runtime_error("[Throttle::ConnectState] Cannot start connection");

					dcclite::Log::Debug("[Throttle::ConnectState] Connecting to {}", serverAddress);
				}

				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time)
				{
					auto status = m_clSocket.GetConnectionProgress();
					if (status == dcclite::Socket::Status::DISCONNECTED)
					{
						self.GotoErrorState(fmt::format("[Throttle::ConnectState] Disconnected on GetConnectionProgress"));
					}
					else if (status == dcclite::Socket::Status::OK)
					{
						dcclite::Log::Debug("[Throttle::ConnectState] Connected to server, starting handshake");

						self.GotoHandShakeState();
					}
				}

				dcclite::Socket m_clSocket;
		};

		struct HandShakeState : State
		{
			HandShakeState(dcclite::Socket socket) :
				m_clSocket{ std::move(socket) }
			{
				//empty
			}

			void Update(Throttle &self, const dcclite::Clock::TimePoint_t time)
			{
				char buffer[6];

				if (!m_fGotVersion)
				{
					auto [status, size] = m_clSocket.Receive(buffer, 5);
					if (status == dcclite::Socket::Status::WOULD_BLOCK)
						return;

					if (status == dcclite::Socket::Status::DISCONNECTED)
					{
						self.GotoErrorState(fmt::format("[Throttle::HandShakeState] Disconnected on reading version"));

						return;
					}

					//got something
					if (strncmp(buffer, "VN2.0", 5))
					{
						//zero terminate, so we can log it
						buffer[5] = '\0';
						
						self.GotoErrorState(fmt::format("[Throttle::HandShakeState] Expected VN2.0 in incomming buffer, but got {}", buffer));

						return;
					}

					m_fGotVersion = true;
				}
								
				//stupid JMRI does not have a consistent line termination, so we try to detect it...
				//it can be \n or \r or \r\n
				if (!m_fLookForward)
				{
					auto [status, size] = m_clSocket.Receive(buffer, 1);
					if (status == dcclite::Socket::Status::WOULD_BLOCK)
						return;

					if (status == dcclite::Socket::Status::DISCONNECTED)
					{
						self.GotoErrorState(fmt::format("[Throttle::HandShakeState] Disconnected on reading LookForward"));

						return;
					}

					//we got something, lets check
					if (buffer[0] == '\n')
					{						
						self.GotoConfiguringThrottleIdState("\n");
						
						return;
					}
					else if (buffer[0] == '\r')
					{
						//try again later, because it can be \r or \r\n, so we must look one byte forward...
						m_fLookForward = true;						
					}
					else
					{												
						self.GotoErrorState(fmt::format("[Throttle::HandShakeState] Invalid line termination, got {:x}", buffer[0]));

						return;
					}
				}
				else
				{
					auto [status, size] = m_clSocket.Receive(buffer, 1);
					if (status == dcclite::Socket::Status::WOULD_BLOCK)
						return;

					if (status == dcclite::Socket::Status::DISCONNECTED)
					{
						self.GotoErrorState(fmt::format("[Throttle::HandShakeState] Disconnected when reading LookForward"));

						return;
					}

					if (buffer[0] == '\n')
					{
						self.GotoConfiguringThrottleIdState("\r\n");
					}
					else
					{
						//not a separator, so it should be some input, add it to the messenger buffer and finish handshake
						buffer[1] = '\0';
						self.GotoConfiguringThrottleIdState("\r", buffer);
					}
				}				
			}

			dcclite::Socket m_clSocket;
			bool m_fGotVersion = false;
			bool m_fLookForward = false;
		};

		struct OnlineState : State
		{
			protected:
				dcclite::NetMessenger m_clMessenger;

				std::chrono::seconds m_uHeartBeatInterval;
				dcclite::Clock::TimePoint_t m_tNextHeartBeat;

			protected:
				OnlineState(dcclite::NetMessenger &&other) :
					m_clMessenger{ std::move(other) }
				{
					//empty
				}

				OnlineState(OnlineState &&other) :
					m_clMessenger{ std::move(other.m_clMessenger) },
					m_uHeartBeatInterval{ other.m_uHeartBeatInterval },
					m_tNextHeartBeat{ other.m_tNextHeartBeat }
				{
					//empty
				}

			public:				
				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
				{
					using namespace std::chrono_literals;

					if (m_uHeartBeatInterval == 0s)
						return;

					if (m_tNextHeartBeat <= time)
					{
						m_clMessenger.Send("*");

						m_tNextHeartBeat = time + m_uHeartBeatInterval;
					}
				}
		};

		struct ConfiguringThrottleIdState : OnlineState
		{			
			public:				
				ConfiguringThrottleIdState(const Throttle &self, dcclite::NetMessenger &&messenger) :
					OnlineState{ std::move(messenger) }
				{					
					m_clMessenger.Send(fmt::format("HU{}", self.GetName()));
					m_clMessenger.Send(fmt::format("N{} {}", "DCCLite", self.GetName()));					
				}

				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
				{
					for (;;)
					{
						auto [status, message] = m_clMessenger.Poll();
						if (status == dcclite::Socket::Status::WOULD_BLOCK)
							break;

						if (status == dcclite::Socket::Status::DISCONNECTED)
						{
							self.GotoErrorState("[Throttle::ConfiguringState::Update] Disconnected");
							break;
						}

						if (message[0] == '*')
						{
							dcclite::Parser parser{ message.c_str() + 1};

							int heartBeat;
							if(parser.GetNumber(heartBeat) != dcclite::Tokens::NUMBER)
							{
								self.GotoErrorState(fmt::format("[Throttle::ConfiguringState::Update] Expected heartbeat seconds, but got: {}", message));

								break;
							}

							m_uHeartBeatInterval = std::chrono::seconds{ heartBeat };
							m_tNextHeartBeat = time + m_uHeartBeatInterval;

							dcclite::Log::Debug("[Throttle::ConnectState] Heartbeat set to {} (server requested)", m_uHeartBeatInterval);

							//
							//when the heartbeat arrives, we satisfy the config state
							self.GotoConnectedState();

							//we will be dead, so return...
							return;
						}
						else if (message[0] == 'H')
						{
							switch (message[1])
							{
								case 'M':
									dcclite::Log::Error("[Throttle::ConfiguringState]{} got error message from server: {}", self.GetName(), message.c_str() + 2);
									break;

								case 'm':
									dcclite::Log::Info("[Throttle::ConfiguringState]{} server info: {}", self.GetName(), message.c_str() + 2);
									break;

								case 'T':
								case 't':
									dcclite::Log::Trace("[Throttle::ConfiguringState]{} server type message: {}", self.GetName(), message.c_str() + 2);
									break;

								default:
									dcclite::Log::Error("[Throttle::ConfiguringState]{} server unknown message: {}", self.GetName(), message);
									break;
							}
						}						
						else if (message.compare(0, 3, "PFT") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} fast clock: {}", self.GetName(), message.c_str() + 3);
						}
						else if (message.compare(0, 3, "PPA") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} track power: {}", self.GetName(), message[3] == '1' ? "ON" : "OFF");
						}
						else if (message.compare(0, 3, "PRL") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} routes list message: {}", self.GetName(), message.c_str() + 3);
						}
						else if (message.compare(0, 3, "PRT") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} routes types message: {}", self.GetName(), message.c_str() + 3);
						}
						else if (message.compare(0, 3, "PTL") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} turnout list message: {}", self.GetName(), message.c_str() + 3);
						}
						else if (message.compare(0, 3, "PTT") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} turnout labels message: {}", self.GetName(), message.c_str() + 3);
						}
						else if (message.compare(0, 2, "PW") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} web port: {}", self.GetName(), message.c_str() + 2);
						}
						else if (message.compare(0, 2, "RC") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} consist info message: {}", self.GetName(), message.c_str() + 2);
						}
						else if (message.compare(0, 2, "RL") == 0)
						{
							dcclite::Log::Trace("[Throttle::ConfiguringState]{} roster list message: {}", self.GetName(), message.c_str() + 2);
						}
						else
						{
							dcclite::Log::Error("[Throttle::ConfiguringState] {} Unknown response: {}", self.GetName(), message);
						}
					}					
				}
		};

		struct ConnectedState: OnlineState
		{			
			public:
				ConnectedState(Throttle &self, OnlineState &&other) :
					OnlineState{std::move(other)},
					m_tPreviousFunctions{ self.m_rclOwnerSlot.GetFunctions() }
				{
					auto locoAddress = self.m_rclOwnerSlot.GetLocomotiveAddress().GetAddress();
					auto locoId = fmt::format("{}{}", locoAddress < 128 ? 'S' : 'L', locoAddress);

					m_clMessenger.Send(fmt::format("MT+{0}<;>{0}", locoId));					 
				}

				void SetSpeed(std::uint8_t speed)
				{
					m_clMessenger.Send(fmt::format("MTA*<;>V{}", speed));
				}

				void SetForward(bool forward)
				{
					m_clMessenger.Send(fmt::format("MTA*<;>R{}", forward ? '1' : '0'));
				}

				void SetFunction(int index, bool pushed)
				{
					const auto message = fmt::format("MTA*<;>f{}{:02}", pushed ? 1 : 0, index);
					m_clMessenger.Send(message);

					dcclite::Log::Debug("[Throttle::ConnectedState] Sent: {}", message);
				}

				void OnFunctionChange(uint8_t beginIndex, uint8_t endIndex, const dcclite::broker::LoconetSlotFunctions_t &functions)
				{
					for (; beginIndex != endIndex; ++beginIndex)
					{
						if (functions[beginIndex] != m_tPreviousFunctions[beginIndex])
						{
							const bool bitValue = functions[beginIndex];

							m_tPreviousFunctions.SetBitValue(beginIndex, bitValue);
							this->SetFunction(beginIndex, bitValue);
						}
					}
				}

				void OnEmergencyStop()
				{
					m_clMessenger.Send("MTA*<;>X");
				}

				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
				{
					for(;;)
					{
						auto [status, message] = m_clMessenger.Poll();

						if (status == dcclite::Socket::Status::DISCONNECTED)
						{
							self.GotoConnectState();
							break;
						}
						else if (status == dcclite::Socket::Status::WOULD_BLOCK)
							break;

						if (message.compare(0, 2, "MT") == 0)
						{
							//multithrottle response
							dcclite::Log::Debug("[Throttle::ConfiguringState] {} {}", self.GetName(), message);
						}
						else
						{
							dcclite::Log::Error("[Throttle::ConnectedState] {} Unknown response: {}", self.GetName(), message);
						}						
					}
					
				}

			private:
				dcclite::broker::LoconetSlotFunctions_t m_tPreviousFunctions;
		};

		std::variant< ConnectState, HandShakeState, ConfiguringThrottleIdState, ConnectedState, ErrorState> m_vState;
		State									*m_pclCurrentState = nullptr;
		ConnectedState							*m_pclConnectedState = nullptr;

		dcclite::broker::DccAddress				m_tLocomotiveAddress;

		const dcclite::NetworkAddress			m_clServerAddress;
		
		const dcclite::broker::ILoconetSlot		&m_rclOwnerSlot;		
};


namespace dcclite::broker
{
	///////////////////////////////////////////////////////////////////////////////
	//
	// ThrottleServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	class ThrottleServiceImpl : public ThrottleService
	{
		public:
			ThrottleServiceImpl(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~ThrottleServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;

			static std::unique_ptr<Service> Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			void Serialize(JsonOutputStream_t &stream) const override;	

			IThrottle &CreateThrottle(const ILoconetSlot &owner) override;
			void ReleaseThrottle(IThrottle &throttle) override;

		private:
			dcclite::NetworkAddress m_clServerAddress;
	};


	ThrottleServiceImpl::ThrottleServiceImpl(const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
		ThrottleService(name, broker, params, project),		
		m_clServerAddress{ dcclite::NetworkAddress::ParseAddress(params["serverAddress"].GetString()) }
	{				
		dcclite::Log::Info("[ThrottleServiceImpl] Started, server at {}", m_clServerAddress);		
	}
	

	ThrottleServiceImpl::~ThrottleServiceImpl()
	{
		//empty
	}

	void ThrottleServiceImpl::Update(const dcclite::Clock& clock)
	{	
		auto enumerator = this->GetEnumerator();
		while (enumerator.MoveNext())
		{
			auto throttle = enumerator.TryGetCurrent<Throttle>();

			throttle->Update(clock);
		}
	}

	void ThrottleServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		ThrottleService::Serialize(stream);		
	}	

	ThrottleService::ThrottleService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)
	{
		//empty
	}

	std::unique_ptr<Service> ThrottleService::Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<ThrottleServiceImpl>(name, broker, params, project);
	}


	IThrottle &ThrottleServiceImpl::CreateThrottle(const ILoconetSlot &owner)
	{
		auto throttle = dynamic_cast<IThrottle *>(this->AddChild(std::make_unique<Throttle>(this->m_clServerAddress, owner )));

		return *throttle;
	}

	void ThrottleServiceImpl::ReleaseThrottle(IThrottle &throttle)
	{
		auto &t = dynamic_cast<Throttle &>(throttle);
		this->RemoveChild(t.GetName());
	}


}