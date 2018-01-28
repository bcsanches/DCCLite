#pragma once

#include "Service.h"

class DccLiteService : public Service
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params);

		virtual ~DccLiteService();
};
