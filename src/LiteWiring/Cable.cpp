#include "Cable.h"

#include <fmt/format.h>

Cable::Cable(Project& project, IntId_t id, std::string name, const DevicePort& source, const DevicePort& sink):
	NamedProjectItem(project, id, std::move(name)),
	m_rclSource(source),
	m_rclSink(sink)
{
	if (m_rclSource.GetNetworkType() != m_rclSink.GetNetworkType())
	{
		throw std::logic_error(
			fmt::format("Cannot create cable {}, incompatible network types for ports: {} vs {}",
				this->GetName(),
				m_rclSource.GetNetworkType().GetName(),
				m_rclSink.GetNetworkType().GetName()
			)
		);
	}
}
