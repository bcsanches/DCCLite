#pragma once

#include <string>

#include "ClassInfo.h"
#include "Object.h"

#include "json.hpp"

class Project;
class Service;

typedef dcclite::ClassInfo<Service, const std::string&, const nlohmann::json &, const Project &> ServiceClass;

namespace dcclite
{
	class Clock;
}

class Service: public dcclite::FolderObject
{
	private:		
		const ServiceClass &m_rclServiceClass;

	protected:
		Service(const ServiceClass &serviceClass, std::string name, const nlohmann::json &params, const Project &project):
			FolderObject(std::move(name)),
			m_rclServiceClass(serviceClass)
		{
			//empty
		}

		Service(const Service &) = delete;
		Service(Service &&) = delete;

		virtual const char *GetTypeName() const noexcept
		{
			return "Service";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			FolderObject::Serialize(stream);

			//nothing
		}

	public:
		virtual ~Service() {}		

		virtual void Update(const dcclite::Clock &clock) { ; }

};

