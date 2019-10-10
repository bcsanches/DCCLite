#pragma once

#include <map>
#include <memory>
#include <string>

#include "Device.h"
#include "LiteWiring.h"
#include "ProjectItem.h"

class Project
{
	public:
		~Project() = default;

		void Load(const std::string &fileName);

	private:
		void Clear();

	private:
		std::map<IntId_t, std::unique_ptr<Device>> m_mapDevices;
		std::map<IntId_t, std::unique_ptr<DeviceType>> m_mapDeviceTypes;
		std::map<IntId_t, std::unique_ptr<NetworkType>> m_mapNetworkTypes;
	
};