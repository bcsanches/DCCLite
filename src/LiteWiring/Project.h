#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Device.h"
#include "LiteWiring.h"
#include "ProjectItem.h"

class Project
{
	public:
		~Project() = default;

		void Load(std::string fileName);
		void Save();

		bool HasName() const
		{
			return !m_strFileName.empty();
		}

		DeviceType *TryAddDeviceType(std::string_view name);

		std::vector<const Device *> GetDevices() const;
		std::vector<const DeviceType *> GetDeviceTypes() const;
		std::vector<const NetworkType *> GetNetworkTypes() const;

	private:
		void Clear();

	private:
		std::string m_strFileName;
		std::string m_strName;

		std::map<std::string, std::unique_ptr<Device>> m_mapDevices;
		std::map<std::string, std::unique_ptr<DeviceType>> m_mapDeviceTypes;
		std::map<std::string, std::unique_ptr<NetworkType>> m_mapNetworkTypes;
	
};