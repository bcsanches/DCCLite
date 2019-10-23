#pragma once

#include <string>

#include "LiteWiring.h"
#include "ProjectItem.h"
#include "Device.h"

class Cable : public NamedProjectItem
{
	public:
		Cable(Project& project, IntId_t id, std::string name, const DevicePort& source, const DevicePort *sink);

	private:
		IntId_t				m_tId;
		const DevicePort	&m_rclSource;
		const DevicePort	*m_pclSink;
};
