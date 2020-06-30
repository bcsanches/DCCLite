#include "LocationManagerService.h"

#include <Log.h>

using namespace dcclite;

static ServiceClass locationManagerService("LocationManagerService",
	[](const ServiceClass& serviceClass, const std::string& name, Broker& broker, const rapidjson::Value& params, const Project& project) ->
	std::unique_ptr<Service> { return std::make_unique<LocationManagerService>(serviceClass, name, broker, params, project); }
);

LocationManagerService::LocationManagerService(const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
	Service(serviceClass, name, broker, params, project)
{
	//empty
}

void LocationManagerService::Initialize()
{
	//empty
}

void LocationManagerService::Update(const dcclite::Clock& clock)
{
	//tick tock
}
