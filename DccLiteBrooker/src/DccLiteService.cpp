#include "DccLiteService.h"

static ServiceClass dccLiteService("DccLite", 
	[](const ServiceClass &serviceClass, const std::string &name) -> std::unique_ptr<Service> { return std::make_unique<DccLiteService>(serviceClass, name); }
);

DccLiteService::DccLiteService(const ServiceClass &serviceClass, const std::string &name) :
	Service(serviceClass, name)
{
	//empty
}

DccLiteService::~DccLiteService()
{
	//empty
}


