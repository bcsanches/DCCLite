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

			IntId_t id = 1;
			for (auto& networkData : networksData.GetArray())
			{
				const char* name = networkData.GetString();

				m_mapNetworkTypes.insert(
					std::make_pair(
						id,
						std::make_unique<NetworkType>(*this, id, name)
					)
				);

				++id;
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
				auto devType = std::make_unique<DeviceType>(*this, deviceTypeData);
				m_mapDeviceTypes.insert(
					std::make_pair(
						devType->GetId(),
						std::move(devType)
					)
				);
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

				devTypeIt.second->Save(devTypeObj);
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