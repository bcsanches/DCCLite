// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <string>

#include "ClassInfo.h"
#include "Object.h"

#include <rapidjson/document.h>

class Broker;
class Project;
class Service;

typedef dcclite::ClassInfo<Service, const std::string&, Broker &, const rapidjson::Value &, const Project &> ServiceClass;

namespace dcclite
{
	class Clock;
}

class Service: public dcclite::FolderObject
{
	public:
		virtual void Initialize() {};

		virtual ~Service() {}

		virtual void Update(const dcclite::Clock& clock) { ; }
	
	protected:
		Service(const ServiceClass &serviceClass, std::string name, Broker &broker, const rapidjson::Value &params, const Project &project):
			FolderObject(std::move(name)),
			m_rclServiceClass(serviceClass),
			m_rclBroker(broker),
			m_rclProject(project)
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

	protected:
		const ServiceClass& m_rclServiceClass;

		Broker& m_rclBroker;

		const Project &m_rclProject;
};

