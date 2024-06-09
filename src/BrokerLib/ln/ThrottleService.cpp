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

#include "../sys/Thinker.h"

#include "Clock.h"
#include "FmtUtils.h"
#include "LoconetService.h"
#include "NetMessenger.h"
#include "Parser.h"

using namespace std::chrono_literals;

///////////////////////////////////////////////////////////////////////////////
//
// Throttle
//
///////////////////////////////////////////////////////////////////////////////


class Throttle: public dcclite::Object, public dcclite::broker::IThrottle
{
	public:
#if 1
		Throttle(const dcclite::NetworkAddress &serverAddress, const dcclite::broker::ILoconetSlot &owner) :
			Object(dcclite::RName{ fmt::format("slot[{}][{}]", owner.GetId(), owner.GetLocomotiveAddress().GetAddress()) }),
			m_clServerAddress{serverAddress},
			m_vState{ ConnectState {serverAddress} },			
			m_rclOwnerSlot{ owner }
		{
			m_pclCurrentState = &std::get<ConnectState>(m_vState);

			assert(m_pclCurrentState);
		}
#endif

		const char *GetTypeName() const noexcept override
		{
			return "Throttle";
		}

		void Update(const dcclite::Clock::TimePoint_t ticks)
		{
			m_pclCurrentState->Update(*this, ticks);
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

		void AddSlave(const dcclite::broker::ILoconetSlot &slot) override
		{
			if (slot.GetLocomotiveAddress() == m_rclOwnerSlot.GetLocomotiveAddress())
			{
				throw std::invalid_argument(fmt::format("[Throttle::AddSlave] Loco {} is the throttle owner", m_rclOwnerSlot.GetLocomotiveAddress()));
			}

			auto it = std::find_if(m_vecSlaves.begin(), m_vecSlaves.end(), [&slot](const dcclite::broker::ILoconetSlot *item) { return slot.GetLocomotiveAddress() == item->GetLocomotiveAddress(); });
			if (it != m_vecSlaves.end())
			{
				throw std::invalid_argument(fmt::format("[Throttle::AddSlave] Loco {} is already on slaves list", slot.GetLocomotiveAddress()));
			}

			m_vecSlaves.push_back(&slot);
			if (!m_pclConnectedState)
				return;

			m_pclConnectedState->OnAddSlave(slot);
		}

		void RemoveSlave(const dcclite::broker::ILoconetSlot &slot) override
		{
			if (slot.GetLocomotiveAddress() == m_rclOwnerSlot.GetLocomotiveAddress())
			{
				throw std::invalid_argument(fmt::format("[Throttle::RemoveSlave] Loco {} is the throttle owner", m_rclOwnerSlot.GetLocomotiveAddress()));
			}

			auto removedIt = std::remove_if(
				m_vecSlaves.begin(),
				m_vecSlaves.end(),
				[&slot](const dcclite::broker::ILoconetSlot *item) { return slot.GetLocomotiveAddress() == item->GetLocomotiveAddress(); }
			);

			if (removedIt == m_vecSlaves.end())
				return;

			if (m_pclConnectedState)
				m_pclConnectedState->OnRemoveSlave(slot);
			
			m_vecSlaves.erase(removedIt, m_vecSlaves.end());
		}

		bool HasSlaves() const noexcept override
		{
			return !m_vecSlaves.empty();
		}

	private:
		template <typename T, class... Args>
		void SetState(Args&&...args)
		{
			m_vState.emplace<T>(static_cast<Args &&>(args)...);
			m_pclCurrentState = &std::get<T>(m_vState);

			m_pclConnectedState = nullptr;
		}

		void GotoConnectState()
		{
			this->SetState<ConnectState>(m_clServerAddress);
		}

		void GotoErrorState(std::string reason)
		{
			this->SetState<ErrorState>(std::move(reason));
		}

		void GotoHandShakeState()
		{
			auto connectingState = std::get_if<ConnectState>(&m_vState);

			if (!connectingState)
				throw std::logic_error("[Throttle::GotoHandShakeState] Invalid state, must be in ConnectingState state");

			this->SetState<HandShakeState>(std::move(connectingState->m_clSocket) );
		}

		void GotoConfiguringThrottleIdState(const char *separator, const char *initialBuffer = "")
		{
			auto handShakeState = std::get_if<HandShakeState>(&m_vState);

			if (!handShakeState)
				throw std::logic_error("[Throttle::GotoConfiguringThrottleIdState] Invalid state, must be in handShakeState state");
			
			dcclite::Log::Debug("[Throttle::GotoConfiguringState] Detected line ending as {} - entering configuring state", separator[0] == '\n' ? "\\n" : separator[1] ? "\\r\\n" : "\\r");
			
			this->SetState<ConfiguringThrottleIdState>( *this, dcclite::NetMessenger{std::move(handShakeState->m_clSocket), separator, initialBuffer});
		}

		void GotoConnectedState()
		{
			auto configuringState = std::get_if<ConfiguringThrottleIdState>(&m_vState);

			if (!configuringState)
				throw std::logic_error("[Throttle::GotoConnectedState] Invalid state, must be in ConfiguringState state");

			auto tmp = std::move(*configuringState);

			this->SetState<ConnectedState>(*this, std::move(tmp));
			m_pclConnectedState = &std::get<ConnectedState>(m_vState);	
		}

	private:		
		struct State
		{
			virtual void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) = 0;
		};

		struct ErrorState : State
		{
			explicit ErrorState(const std::string &reason)
			{
				dcclite::Log::Error(reason);
			}

			void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
			{
				//empty
			}
		};

		struct ConnectState: State
		{							
			public:
				explicit ConnectState(const dcclite::NetworkAddress &serverAddress)
				{
					if (!m_clSocket.StartConnection(0, dcclite::Socket::Type::STREAM, serverAddress))
						throw std::runtime_error("[Throttle::ConnectState] Cannot start connection");

					dcclite::Log::Debug("[Throttle::ConnectState] Connecting to {}", serverAddress);
				}

				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
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
			explicit HandShakeState(dcclite::Socket socket) :
				m_clSocket{ std::move(socket) }
			{
				//empty
			}

			void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
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
				explicit OnlineState(dcclite::NetMessenger &&other) :
					m_clMessenger{ std::move(other) }
				{
					//empty
				}

				OnlineState(OnlineState &&other) noexcept :
					m_clMessenger{ std::move(other.m_clMessenger) },
					m_uHeartBeatInterval{ other.m_uHeartBeatInterval },
					m_tNextHeartBeat{ other.m_tNextHeartBeat }
				{
					//empty
				}				

			protected:
				static bool ParseMessage(Throttle &self, const std::string &message)
				{
					if (message[0] == 'H')
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
						return false;
					}

					return true;
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

							//we are dead now, so return...
							return;
						}
						else if(!OnlineState::ParseMessage(self, message))						
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
					this->RegisterLocomotive(self.m_rclOwnerSlot);

					for (auto &it : self.m_vecSlaves)
					{
						this->RegisterLocomotive(*it);
					}
				}

				void SetSpeed(std::uint8_t speed)
				{
					m_clMessenger.Send(fmt::format("MTA*<;>V{}", speed));
				}

				void SetForward(bool forward)
				{
					m_clMessenger.Send(fmt::format("MTA*<;>R{}", forward ? '1' : '0'));
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
						else if (!OnlineState::ParseMessage(self, message))
						{
							dcclite::Log::Error("[Throttle::ConfiguringState] {} Unknown response: {}", self.GetName(), message);
						}						
					}
					
				}

				void OnAddSlave(const dcclite::broker::ILoconetSlot &slot)
				{
					this->RegisterLocomotive(slot);
				}

				void OnRemoveSlave(const dcclite::broker::ILoconetSlot &slot)
				{
					this->UnregisterLocomotive(slot);
				}

			private:
				static std::string MakeLocoId(const dcclite::broker::ILoconetSlot &slot)
				{
					auto locoAddress = slot.GetLocomotiveAddress().GetAddress();
					return fmt::format("{}{}", locoAddress < 128 ? 'S' : 'L', locoAddress);
				}

				void RegisterLocomotive(const dcclite::broker::ILoconetSlot &slot)
				{				
					m_clMessenger.Send(fmt::format("MT+{0}<;>{0}", MakeLocoId(slot)));
				}

				void UnregisterLocomotive(const dcclite::broker::ILoconetSlot &slot)
				{
					m_clMessenger.Send(fmt::format("MT-{}<;>r", MakeLocoId(slot)));
				}

				void SetFunction(int index, bool pushed)
				{
					const auto message = fmt::format("MTA*<;>f{}{:02}", pushed ? 1 : 0, index);
					m_clMessenger.Send(message);

					dcclite::Log::Debug("[Throttle::ConnectedState] Sent: {}", message);
				}

			private:
				dcclite::broker::LoconetSlotFunctions_t m_tPreviousFunctions;
		};

		std::variant< ConnectState, HandShakeState, ConfiguringThrottleIdState, ConnectedState, ErrorState> m_vState;
		State									*m_pclCurrentState = nullptr;
		ConnectedState							*m_pclConnectedState = nullptr;		

		const dcclite::NetworkAddress			m_clServerAddress;
		
		const dcclite::broker::ILoconetSlot		&m_rclOwnerSlot;		

		std::vector<const dcclite::broker::ILoconetSlot *> m_vecSlaves;
};


namespace dcclite::broker
{
	///////////////////////////////////////////////////////////////////////////////
	//
	// ThrottleServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	static const char *DetermineServerAddress(const rapidjson::Value &params)
	{
		auto it = params.FindMember("serverAddress");
		if (it == params.MemberEnd())
			return "127.0.0.1:12090";
		else
			return it->value.GetString();
	}

	class ThrottleServiceImpl : public ThrottleService
	{
		public:
			ThrottleServiceImpl(RName name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~ThrottleServiceImpl() override;			

			void Serialize(JsonOutputStream_t &stream) const override;	

			IThrottle &CreateThrottle(const ILoconetSlot &owner) override;
			void ReleaseThrottle(IThrottle &throttle) override;

		private:
			void Think(const dcclite::Clock::TimePoint_t ticks);

		private:
			Thinker m_tThinker;

			dcclite::NetworkAddress m_clServerAddress;
			unsigned int			m_uThrottleCount = 0;
	};


	ThrottleServiceImpl::ThrottleServiceImpl(RName name, Broker &broker, const rapidjson::Value& params, const Project& project):
		ThrottleService(name, broker, params, project),		
		m_clServerAddress{ dcclite::NetworkAddress::ParseAddress(DetermineServerAddress(params)) },
		m_tThinker{"ThrottleServiceImpl::Thinker", THINKER_MF_LAMBDA(Think)}
	{				
		dcclite::Log::Info("[ThrottleServiceImpl] Started, server at {}", m_clServerAddress);		
	}
	

	ThrottleServiceImpl::~ThrottleServiceImpl()
	{
		//empty
	}

	void ThrottleServiceImpl::Think(const dcclite::Clock::TimePoint_t ticks)
	{	
		m_tThinker.Schedule(ticks + 20ms);

		auto enumerator = this->GetEnumerator();
		while (enumerator.MoveNext())
		{
			auto throttle = enumerator.GetCurrent<Throttle>();

			throttle->Update(ticks);
		}
	}

	void ThrottleServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		ThrottleService::Serialize(stream);		
	}	

	ThrottleService::ThrottleService(RName name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)
	{
		//empty
	}

	std::unique_ptr<Service> ThrottleService::Create(RName name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<ThrottleServiceImpl>(name, broker, params, project);
	}


	IThrottle &ThrottleServiceImpl::CreateThrottle(const ILoconetSlot &owner)
	{
		auto throttle = dynamic_cast<IThrottle *>(this->AddChild(std::make_unique<Throttle>(this->m_clServerAddress, owner )));

		if (!m_tThinker.IsScheduled())
			m_tThinker.Schedule({});

		++m_uThrottleCount;

		return *throttle;
	}

	void ThrottleServiceImpl::ReleaseThrottle(IThrottle &throttle)
	{
		auto &t = dynamic_cast<Throttle &>(throttle);
		this->RemoveChild(t.GetName());		

		--m_uThrottleCount;
		if (!m_uThrottleCount)
		{
			//nothing else to do... so go to sleep
			m_tThinker.Cancel();
		}
	}
}
