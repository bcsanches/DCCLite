#pragma once

#include <map>
#include <memory>
#include <string>

#include <rapidjson/document.h>

#include "LiteWiring.h"
#include "ProjectItem.h"

class DeviceType;

class DeviceModel
{
	public:
		DeviceModel(DeviceType& owner, std::string name) :
			m_strName(std::move(name)),
			m_rclType(owner)
		{
			//empty
		}

		const std::string& GetName() const
		{
			return m_strName;
		}

	private:	
		std::string m_strName;
		DeviceType& m_rclType;
};

class DeviceType: public NamedProjectItem
{
	public:
		DeviceType(Project& project, std::string name) :
			NamedProjectItem(project, std::move(name))
		{
			//empty
		}			

		std::vector<const DeviceModel *> GetDeviceModels() const
		{
			return detail::FillVector<DeviceModel>(m_mapModels);
		}

		bool TryAddModel(std::string_view name);

	private:
		std::map<std::string, std::unique_ptr<DeviceModel>> m_mapModels;
};



class DevicePort
{
	public:
		DevicePort(std::string name, const NetworkType& networkType) :
			m_strName(std::move(name)),
			m_rclNetworkType(networkType)
		{
			//empty
		}

		const NetworkType& GetNetworkType() const
		{
			return m_rclNetworkType;
		}

	private:
		std::string m_strName;
		const NetworkType& m_rclNetworkType;
};

class Device: public NamedProjectItem
{	
	public:
		Device(Project& project, std::string name, const DeviceModel& model) :
			NamedProjectItem(project, std::move(name)),
			m_pclModel(&model)
		{
			//empty
		}

		const DevicePort& AddPort(std::string name, const NetworkType& networkType);

		const DevicePort* TryFindPort(const std::string_view name);

	private:						
		const DeviceModel *m_pclModel;
};
