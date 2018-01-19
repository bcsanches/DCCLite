#include "TerminalService.h"

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name) -> std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name); }
);

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name) :
	Service(serviceClass, name)
{
	//empty
}

TerminalService::~TerminalService()
{
	//empty
}

