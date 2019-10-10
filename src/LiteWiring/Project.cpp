#include "Project.h"

#include <fmt/format.h>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "Cable.h"
#include "Device.h"

void Project::Load(const std::string &fileName)
{
	std::ifstream dataFile(fileName.c_str());

	if (!dataFile)
	{
		throw std::runtime_error(fmt::format("error: cannot open config file {}", fileName));
	}

	rapidjson::IStreamWrapper isw(dataFile);
	rapidjson::Document data;
	data.ParseStream(isw);

	this->Clear();

	try
	{
		const auto& networksData = data["networks"];
		if (!networksData.IsArray())
		{
			throw std::runtime_error("error: [Project::Load] invalid config, expected networks array");
		}

		IntId_t id = 1;
		for (auto& networkData : networksData.GetArray())
		{
			const char *name = networkData.GetString();

			m_mapNetworkTypes.insert(
				std::make_pair(
					id,
					std::make_unique<NetworkType>(*this, id, name)
				)
			);

			++id;
		}

		const auto& deviceTypesData = data["deviceTypes"];
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
	catch(...)
	{
		this->Clear();

		throw;
	}
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