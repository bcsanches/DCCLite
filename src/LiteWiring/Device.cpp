#include "Device.h"

#include <utility>

DeviceType::DeviceType(Project& project, const rapidjson::Value& params):
	NamedProjectItem(project, params["id"].GetInt(), params["name"].GetString())
{
	auto& modelsData = params["models"];
	if (!modelsData.IsArray())
	{
		throw std::runtime_error("error: [DeviceType] expected models array");
	}

	IntId_t id = 1;
	for (auto& modelData : modelsData.GetArray())
	{
		const char* name = modelData["name"].GetString();

		m_mapModels.insert(
			std::make_pair(
				id,
				std::make_unique<DeviceModel>(*this, name)
			)
		);

		++id;
	}
}
