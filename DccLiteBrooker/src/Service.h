#pragma once

#include <string>

#include "ClassInfo.h"

#include "json.hpp"

class Service;
typedef dcclite::ClassInfo<Service, const std::string&, const nlohmann::json &> ServiceClass;

class Service
{
	private:
		std::string m_strName;

		const ServiceClass &m_rclServiceClass;

	protected:
		Service(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params):
			m_rclServiceClass(serviceClass),
			m_strName(name)
		{
			//empty
		}

		Service(const Service &) = delete;
		Service(Service &&) = delete;

	public:
		virtual ~Service() {}

		const std::string &GetName() { return m_strName; }

		virtual void Update() { ; }

};

