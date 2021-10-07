#include "ThrottleService.h"

#include <Log.h>

#include <variant>

#include "Clock.h"
#include "FmtUtils.h"
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
		Throttle(const dcclite::NetworkAddress &serverAddress, dcclite::broker::DccAddress locomotiveAddress) :
			IObject(std::move(locomotiveAddress.ToString())),
			m_clServerAddress{serverAddress},
			m_vState{ ConnectState {serverAddress} },
			m_tLocomotiveAddress{locomotiveAddress}
		{
			m_pclCurrentState = std::get_if<ConnectState>(&m_vState);
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

	private:
		void GotoConnectState()
		{
			m_vState = ConnectState{ m_clServerAddress };
			m_pclCurrentState = std::get_if<ErrorState>(&m_vState);
		}

		void GotoErrorState(std::string reason)
		{
			m_vState = ErrorState{std::move(reason)};
			m_pclCurrentState = std::get_if<ErrorState>(&m_vState);
		}

		void GotoHandShakeState()
		{
			auto connectingState = std::get_if<ConnectState>(&m_vState);

			if (!connectingState)
				throw std::logic_error("[Throttle::GotoHandShakeState] Invalid state, must be in ConnectingState state");

			m_vState = HandShakeState{ std::move(connectingState->m_clSocket) };
			m_pclCurrentState = std::get_if<HandShakeState>(&m_vState);
		}

		void GotoConfiguringState(const char *separator, const char *initialBuffer = "")
		{
			auto handShakeState = std::get_if<HandShakeState>(&m_vState);

			if (!handShakeState)
				throw std::logic_error("[Throttle::GotoConfiguringState] Invalid state, must be in handShakeState state");
			
			m_vState.emplace<ConfiguringState>( *this, dcclite::NetMessenger{std::move(handShakeState->m_clSocket), separator, initialBuffer});
			m_pclCurrentState = std::get_if<ConfiguringState>(&m_vState);
		}

		void GotoConnectedState()
		{
			auto configuringState = std::get_if<ConfiguringState>(&m_vState);

			if (!configuringState)
				throw std::logic_error("[Throttle::GotoConnectedState] Invalid state, must be in ConfiguringState state");

			m_vState.emplace<ConnectedState>(std::move(*configuringState));
			m_pclCurrentState = std::get_if<ConnectedState>(&m_vState);
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
						self.GotoConfiguringState("\n");
						
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
						self.GotoConfiguringState("\r\n");
					}
					else
					{
						//not a separator, so it should be some input, add it to the messenger buffer and finish handshake
						buffer[1] = '\0';
						self.GotoConfiguringState("\r", buffer);
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

		struct ConfiguringState : OnlineState
		{			
			public:				
				ConfiguringState(const Throttle &self, dcclite::NetMessenger &&messenger) :
					OnlineState{ std::move(messenger) }
				{
					m_clMessenger.Send(fmt::format("H{}", fmt::ptr(&self)));
					m_clMessenger.Send(fmt::format("N{} {}", "DCCLite", fmt::ptr (&self)));
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
						}

					}
					
				}
		};

		struct ConnectedState: OnlineState
		{			
			public:

				ConnectedState(OnlineState &&other) :
					OnlineState{std::move(other)}
				{
					//empty
				}

				void Update(Throttle &self, const dcclite::Clock::TimePoint_t time) override
				{
					for(;;)
					{
						auto [status, msg] = m_clMessenger.Poll();

						if (status == dcclite::Socket::Status::DISCONNECTED)
						{
							self.GotoConnectState();
							break;
						}
						else if (status == dcclite::Socket::Status::WOULD_BLOCK)
							break;

						dcclite::Log::Trace("Got: {}", msg);
					}
					
				}
		};

		std::variant< ConnectState, HandShakeState, ConfiguringState, ConnectedState, ErrorState> m_vState;
		State *m_pclCurrentState = nullptr;

		dcclite::broker::DccAddress m_tLocomotiveAddress;

		const dcclite::NetworkAddress m_clServerAddress;

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

			IThrottle &CreateThrottle(DccAddress locomotiveAddress) override;
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


	IThrottle &ThrottleServiceImpl::CreateThrottle(DccAddress locomotiveAddress)
	{
		auto throttle = dynamic_cast<IThrottle *>(this->AddChild(std::make_unique<Throttle>(this->m_clServerAddress, locomotiveAddress )));

		return *throttle;
	}

	void ThrottleServiceImpl::ReleaseThrottle(IThrottle &throttle)
	{
		auto &t = dynamic_cast<Throttle &>(throttle);
		this->RemoveChild(t.GetName());
	}


}