#include "Cable.h"

#include <fmt/format.h>

Cable::Cable(Project& project, IntId_t id, std::string name, const DevicePort& source, const DevicePort *sink):
	NamedProjectItem(project, std::move(name)),
	m_tId(id),
	m_rclSource(source),
	m_pclSink(sink)
{
	if (m_pclSink && (m_rclSource.GetNetworkType() != m_pclSink->GetNetworkType()))
	{
		throw std::logic_error(
			fmt::format("Cannot create cable {}, incompatible network types for ports: {} vs {}",
				this->GetName(),
				m_rclSource.GetNetworkType().GetName(),
				m_pclSink->GetNetworkType().GetName()
			)
		);
	}
}
