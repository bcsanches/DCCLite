#include "Device.h"

#include <utility>

DeviceType::DeviceType(Project& project, const rapidjson::Value& params):
	NamedProjectItem(project, params["id"].GetInt(), params["name"].GetString())
{
	auto& memberIt = params.FindMember("models");
	if (memberIt == params.MemberEnd())
		return;

	auto& modelsData = memberIt->value;
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

void DeviceType::Save(JsonOutputStream_t& stream)
{
	stream.AddIntValue("id", this->GetId());
	stream.AddStringValue("name", this->GetName());

	if (m_mapModels.empty())
		return;

	auto modelsArray = stream.AddArray("models");

	for (auto& modelIt : m_mapModels)
	{
		auto obj = modelsArray.AddObject();
		obj.AddStringValue("name", modelIt.second->GetName());
	}
}
