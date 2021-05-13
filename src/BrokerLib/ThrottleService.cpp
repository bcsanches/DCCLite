#include "ThrottleService.h"

#include <Log.h>

#include "FmtUtils.h"
#include "NetMessenger.h"

///////////////////////////////////////////////////////////////////////////////
//
// Throttle
//
///////////////////////////////////////////////////////////////////////////////

class Throttle
{
	public:

	private:
		dcclite::NetMessenger m_clMessenger;
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


}