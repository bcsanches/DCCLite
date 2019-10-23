#include "Device.h"

#include <utility>

bool DeviceType::TryAddModel(std::string_view name)
{
	std::string tmpName(name);
	auto it = m_mapModels.find(tmpName);

	if (it != m_mapModels.end())
	{
		return false;
	}

	m_mapModels.insert(std::make_pair(
		std::move(tmpName), 
		std::make_unique<DeviceModel>(
			*this, std::string{name}
		)
	));

	return true;
}