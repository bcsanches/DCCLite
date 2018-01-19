#pragma once

#include <string>

#include "ClassInfo.h"

class Service;
typedef dcclite::ClassInfo<Service, const std::string&> ServiceClass;

class Service
{
	private:
		std::string m_strName;

		const ServiceClass &m_rclServiceClass;

	protected:
		Service(const ServiceClass &serviceClass, const std::string &name):
			m_rclServiceClass(serviceClass),
			m_strName(name)
		{
			//empty
		}

	public:
		virtual ~Service() {}

		const std::string &GetName() { return m_strName; }

};

