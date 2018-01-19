#pragma once

#include "Service.h"

class TerminalService : public Service
{
	public:
		TerminalService(const ServiceClass &serviceClass, const std::string &name);

		virtual ~TerminalService();
};
