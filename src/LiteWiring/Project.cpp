#include "Project.h"

#include <fmt/format.h>
#include <fstream>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "Cable.h"
#include "Device.h"

void Project::Load(std::string fileName)
{	
	std::ifstream dataFile(fileName.c_str());
	m_strFileName = std::move(fileName);

	if (!dataFile)
	{
		throw std::runtime_error(fmt::format("error: cannot open config file {}", m_strFileName));
	}	

	rapidjson::IStreamWrapper isw(dataFile);
	rapidjson::Document data;
	data.ParseStream(isw);

	this->Clear();

	m_strName = data["name"].GetString();

	try
	{
		auto memberIt = data.FindMember("networks");
		if (memberIt != data.MemberEnd())
		{
			const auto& networksData = memberIt->value;
			if (!networksData.IsArray())
			{
				throw std::runtime_error("error: [Project::Load] invalid config, expected networks array");
			}
			
			for (auto& networkData : networksData.GetArray())
			{
				const char* name = networkData.GetString();

				m_mapNetworkTypes.insert(
					std::make_pair(
						std::string{name},
						std::make_unique<NetworkType>(*this, name)
					)
				);
			}
		}
		
		memberIt = data.FindMember("deviceTypes");
		if (memberIt != data.MemberEnd())
		{
			const auto& deviceTypesData = memberIt->value;
			if (!deviceTypesData.IsArray())
			{
				throw std::runtime_error("error: [Project::Load] invalid config, expected networks array");
			}

			for (auto& deviceTypeData : deviceTypesData.GetArray())
			{	
				auto devName = deviceTypeData["name"].GetString();

				auto devType = this->TryAddDeviceType(devName);
				if (devType == nullptr)
				{				
					throw std::runtime_error(fmt::format("error: [Project::Load] Device type {} already exists", devName));
				}
				
				auto &modelsIt = deviceTypeData.FindMember("models");
				if (modelsIt == deviceTypeData.MemberEnd())
				{
					continue;
				}					

				auto &modelsData = memberIt->value;
				if (!modelsData.IsArray())
				{
					throw std::runtime_error(fmt::format("error: [DeviceType::{}] expected models array", devName));
				}

				for (auto &modelData : modelsData.GetArray())
				{
					const char *name = modelData["name"].GetString();

					if (!devType->TryAddModel(name))
					{
						throw std::runtime_error(fmt::format("error: [DeviceType::{}] device model {} already exists", devName, name));
					}
				}				
			}
		}

		memberIt = data.FindMember("devices");
		if (memberIt != data.MemberEnd())
		{
			const auto &devicesData = memberIt->value;
			if (!devicesData.IsArray())
			{
				throw std::runtime_error("error: [Project::Load] invalid config, expected devices array");
			}


		}
	}
	catch(...)
	{
		this->Clear();

		throw;
	}
}

void Project::Save()
{
	if (!this->HasName())
	{
		throw std::logic_error("Cannot save project without a file name, use save as ...");
	}

	std::ofstream outputFile(m_strFileName, std::ios_base::trunc);

	JsonCreator::StringWriter responseWriter;
	{
		auto object = JsonCreator::MakeObject(responseWriter);
		object.AddStringValue("name", m_strName);

		if(!m_mapNetworkTypes.empty())
		{		
			auto& networkArray = object.AddArray("networks");

			for(const auto & networkIt : m_mapNetworkTypes)
			{
				networkArray.AddString(networkIt.second->GetName().c_str());
			}
		}

		if (!m_mapDeviceTypes.empty())
		{
			auto& deviceTypesArray = object.AddArray("deviceTypes");

			for (const auto& devTypeIt : m_mapDeviceTypes)
			{
				auto& devTypeObj = deviceTypesArray.AddObject();

				devTypeObj.AddStringValue("name", devTypeIt.second->GetName());

				const auto models = devTypeIt.second->GetDeviceModels();
				if(models.empty())
					continue;

				auto modelsOutputStream = devTypeObj.AddArray("models");
				for(auto devModel : models)
				{
					auto modelOutputObj = modelsOutputStream.AddObject();
					modelOutputObj.AddStringValue("name", devModel->GetName());
				}				
			}
		}
	}

	outputFile << responseWriter.GetString();
}

void Project::Clear()
{	
	m_mapDevices.clear();
	m_mapDeviceTypes.clear();
	m_mapNetworkTypes.clear();
}

std::vector<const Device *> Project::GetDevices() const
{
	return detail::FillVector<Device>(m_mapDevices);
}

std::vector<const DeviceType *> Project::GetDeviceTypes() const
{
	return detail::FillVector<DeviceType>(m_mapDeviceTypes);
}

std::vector<const NetworkType *> Project::GetNetworkTypes() const
{
	return detail::FillVector<NetworkType>(m_mapNetworkTypes);
}

DeviceType *Project::TryAddDeviceType(std::string_view name)
{
	std::string tmpName(name);
	if (m_mapDeviceTypes.find(tmpName) != m_mapDeviceTypes.end())
	{
		return nullptr;
	}

	auto devType = std::make_unique<DeviceType>(*this, tmpName);

	auto *p = devType.get();

	m_mapDeviceTypes.insert(std::make_pair(std::move(tmpName), std::move(devType)));

	return p;
}

#if 0
void Test()
{
	Project project;
	DeviceType devTypeBooster{ project, 1, "booster" };
	DeviceModel devModelBoosterDB150{ project, 1, "DB150" };	

	NetworkType netLoconet{ project, 1, "Loconet" };

	Device mainBooster{ project, 1, "Main Booster", devTypeBooster, devModelBoosterDB150 };

	auto mainBoosterPortA = mainBooster.AddPort("A", netLoconet);


	DeviceType devTypeDigitraxPanel{ project, 1, "panel" };
	DeviceModel devModelPanelUR92{ project, 1, "UR92" };

	Device panelInfrared{ project, 2, "Infrared Panel", devTypeDigitraxPanel , devModelPanelUR92 };

	auto panelInfraredPortA = panelInfrared.AddPort("A", netLoconet);

	Cable cab{ project, 1, "Ln1", mainBoosterPortA, panelInfraredPortA };
}
#endif