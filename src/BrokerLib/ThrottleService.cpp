#include "ThrottleService.h"

#include <Log.h>

#include <variant>

#include "FmtUtils.h"
#include "NetMessenger.h"

///////////////////////////////////////////////////////////////////////////////
//
// Throttle
//
///////////////////////////////////////////////////////////////////////////////


class Throttle: public dcclite::IObject
{
	public:
#if 1
		Throttle(const dcclite::NetworkAddress &serverAddress, dcclite::broker::DccAddress locomotiveAddress) :
			IObject(std::move(locomotiveAddress.ToString())),
			m_clServerAddress{serverAddress},
			m_vState{ ConnectingState {serverAddress} },
			m_tLocomotiveAddress{locomotiveAddress}
		{
			m_pclCurrentState = std::get_if<ConnectingState>(&m_vState);
		}
#endif

		const char *GetTypeName() const noexcept override
		{
			return "Throttle";
		}

		void Update(const dcclite::Clock &clock)
		{
			m_pclCurrentState->Update(*this);
		}

	private:
		void GotoConnectingState()
		{
			m_vState = ConnectingState{ m_clServerAddress };
			m_pclCurrentState = std::get_if<ErrorState>(&m_vState);
		}

		void GotoErrorState(std::string reason)
		{
			m_vState = ErrorState{std::move(reason)};
			m_pclCurrentState = std::get_if<ErrorState>(&m_vState);
		}

		void GotoHandShakeState()
		{
			auto connectingState = std::get_if<ConnectingState>(&m_vState);

			if (!connectingState)
				throw std::logic_error("[Throttle::GotoHandShakeState] Invalid state, must be in ConnectingState state");

			m_vState = HandShakeState{ std::move(connectingState->m_clSocket) };
			m_pclCurrentState = std::get_if<HandShakeState>(&m_vState);
		}

		void GotoConnectedState(const char *separator, const char *initialBuffer = "")
		{
			auto handShakeState = std::get_if<HandShakeState>(&m_vState);

			if (!handShakeState)
				throw std::logic_error("[Throttle::GotoConnectedState] Invalid state, must be in handShakeState state");

			m_vState = ConnectedState{ std::move(handShakeState->m_clSocket), separator, initialBuffer };
			m_pclCurrentState = std::get_if<ConnectedState>(&m_vState);
		}

	private:		
		struct State
		{
			virtual void Update(Throttle &self) = 0;
		};

		struct ErrorState : State
		{
			ErrorState(std::string reason)
			{
				dcclite::Log::Error(reason.c_str());
			}

			void Update(Throttle &self) override
			{
				//empty
			}
		};

		struct ConnectingState: State
		{							
			public:
				ConnectingState(const dcclite::NetworkAddress &serverAddress)
				{
					if (!m_clSocket.StartConnection(0, dcclite::Socket::Type::STREAM, serverAddress))
						throw std::runtime_error("[Throttle::ConnectingState] Cannot start connection");
				}

				void Update(Throttle &self)
				{
					auto status = m_clSocket.GetConnectionProgress();
					if (status == dcclite::Socket::Status::DISCONNECTED)
					{
						self.GotoErrorState(fmt::format("[Throttle::ConnectingState] Disconnected on GetConnectionProgress"));
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

			void Update(Throttle &self)
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
						self.GotoConnectedState("\n");
						
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
						self.GotoConnectedState("\r\n");
					}
					else
					{
						//not a separator, so it should be some input, add it to the messenger buffer and finish handshake
						buffer[1] = '\0';
						self.GotoConnectedState("\r", buffer);
					}
				}				
			}

			dcclite::Socket m_clSocket;
			bool m_fGotVersion = false;
			bool m_fLookForward = false;
		};

		struct ConnectedState: State
		{
			private:
				dcclite::NetMessenger m_clMessenger;

			public:

				ConnectedState(dcclite::Socket socket, const char *separator, const char *initialBuffer) :
					m_clMessenger{ std::move(socket), separator, initialBuffer }
				{
					//empty
				}

				void Update(Throttle &self)
				{
					for(;;)
					{
						auto [status, msg] = m_clMessenger.Poll();

						if (status == dcclite::Socket::Status::DISCONNECTED)
						{
							self.GotoConnectingState();
							break;
						}
						else if (status == dcclite::Socket::Status::WOULD_BLOCK)
							break;

						dcclite::Log::Trace("Got: {}", msg);
					}
					
				}
		};

		std::variant< ConnectingState, HandShakeState, ConnectedState, ErrorState> m_vState;
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