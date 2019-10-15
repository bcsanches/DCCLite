#pragma once

#include <map>
#include <string>

#include <JsonCreator/Object.h>
#include <JsonCreator/StringWriter.h>

#include <rapidjson/document.h>

#include "LiteWiring.h"
#include "ProjectItem.h"

class DeviceType;

typedef JsonCreator::Object<JsonCreator::StringWriter> JsonOutputStream_t;

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
		DeviceType(Project& project, const IntId_t id, std::string name) :
			NamedProjectItem(project, id, std::move(name))
		{
			//empty
		}	

		DeviceType(Project& project, const rapidjson::Value& params);

		void Save(JsonOutputStream_t &stream);

		std::vector<const DeviceModel *> GetDeviceModels() const
		{
			return detail::FillVector<DeviceModel>(m_mapModels);
		}

	private:
		std::map<IntId_t, std::unique_ptr<DeviceModel>> m_mapModels;
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
		Device(Project& project, const IntId_t id, std::string name, const DeviceModel& model) :
			NamedProjectItem(project, id, std::move(name)),
			m_pclModel(&model)
		{
			//empty
		}

		const DevicePort& AddPort(std::string name, const NetworkType& networkType);

		const DevicePort* TryFindPort(const std::string_view name);

	private:						
		const DeviceModel *m_pclModel;
};
