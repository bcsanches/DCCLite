#pragma once

#include "Service.h"

class DccLiteService : public Service
{
	public:
		DccLiteService(const std::string &name);

		virtual ~DccLiteService();
};
