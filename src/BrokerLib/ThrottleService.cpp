#include "ThrottleService.h"

#include <Log.h>

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
#if 0
		Throttle(std::string name, const dcclite::NetworkAddress &serverAddress) :
			IObject(std::move(name)),
			m_clMessenger()
		{
			//empty
		}
#endif

	private:
		void GotoConnectingState();
		void GotoConnectedState();

	private:
		dcclite::NetMessenger m_clMessenger;

		struct State
		{

		};

		struct ConnectingState: State
		{
			private:
				dcclite::Socket m_clSocket;

			public:
				ConnectingState(const dcclite::NetworkAddress &serverAddress)
				{
					if (!m_clSocket.StartConnection(serverAddress))
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
		};

		struct ConnectedState: State
		{

		};

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

			void Initialize() override;

			static std::unique_ptr<Service> Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			void Serialize(JsonOutputStream_t &stream) const override;	

			std::unique_ptr<IThrottle> CreateThrottle(DccAddress locomotiveAddress) override;

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

	void ThrottleServiceImpl::Initialize()
	{
		//empty
	}		

	void ThrottleServiceImpl::Update(const dcclite::Clock& clock)
	{	
		
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


	std::unique_ptr<IThrottle> ThrottleServiceImpl::CreateThrottle(DccAddress locomotiveAddress)
	{
		return nullptr;
	}


}