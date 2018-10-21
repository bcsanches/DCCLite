#pragma once

#include <string>

#include "ClassInfo.h"
#include "Object.h"

#include "json.hpp"

class Service;
typedef dcclite::ClassInfo<Service, const std::string&, const nlohmann::json &> ServiceClass;

class Service: public dcclite::Object
{
	private:		
		const ServiceClass &m_rclServiceClass;

	protected:
		Service(const ServiceClass &serviceClass, std::string name, const nlohmann::json &params):
			Object(std::move(name)),
			m_rclServiceClass(serviceClass)
		{
			//empty
		}

		Service(const Service &) = delete;
		Service(Service &&) = delete;

	public:
		virtual ~Service() {}		

		virtual void Update() { ; }

};

