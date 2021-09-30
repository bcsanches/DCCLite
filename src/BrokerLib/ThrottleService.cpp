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
		}

		void GotoConnectedState()
		{
			auto connectingState = std::get_if<ConnectingState>(&m_vState);

			if (!connectingState)
				throw std::logic_error("[Throttle::GotoConnectedState] Invalid state, must be in connecting state");

			m_vState = ConnectedState{ std::move(connectingState->m_clSocket) };
			m_pclCurrentState = std::get_if<ConnectedState>(&m_vState);
		}

	private:		
		struct State
		{
			virtual void Update(Throttle &self) = 0;
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
						self.GotoConnectingState();
					}
					else if (status == dcclite::Socket::Status::OK)
					{
						self.GotoConnectedState();
					}
				}

				dcclite::Socket m_clSocket;
		};

		struct ConnectedState: State
		{
			private:
				dcclite::NetMessenger m_clMessenger;

			public:

				ConnectedState(dcclite::Socket socket) :
					m_clMessenger{ std::move(socket), "\n"}
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

		std::variant< ConnectingState, ConnectedState> m_vState;
		State *m_pclCurrentState = nullptr;

		dcclite::broker::DccAddress m_tLocomotiveAddress;

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
		//this->RemoveChild(throttle.GetName());
	}


}