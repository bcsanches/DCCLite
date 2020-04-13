#include "LiteManagerService.h"

#include <Log.h>

using namespace dcclite;

static ServiceClass liteManagerService("LiteManagerService",
	[](const ServiceClass& serviceClass, const std::string& name, Broker& broker, const rapidjson::Value& params, const Project& project) ->
	std::unique_ptr<Service> { return std::make_unique<LiteManagerService>(serviceClass, name, broker, params, project); }
);

LiteManagerService::LiteManagerService(const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
	Service(serviceClass, name, broker, params, project)
{
	//empty
}

void LiteManagerService::Initialize()
{
	//empty
}

void LiteManagerService::Update(const dcclite::Clock& clock)
{
	//tick tock
}
