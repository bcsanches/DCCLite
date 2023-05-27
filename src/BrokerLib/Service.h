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

#include <functional>
#include <string>
#include <vector>

#include "Object.h"

#include <rapidjson/document.h>

#include <sigslot/signal.hpp>

namespace dcclite
{
	class Clock;
}

namespace dcclite::broker
{
	class Broker;
	class Project;
	class Service;


	class ObjectManagerEvent
	{		
		public:

			typedef std::function<void(JsonOutputStream_t &stream)> SerializeDeltaProc_t;

			enum EventType
			{
				ITEM_CREATED,
				ITEM_DESTROYED,

				ITEM_CHANGED
			};


			ObjectManagerEvent(EventType ev, const Service &manager, const IItem *item, SerializeDeltaProc_t serializeDeltaProc = nullptr):
				m_kType(ev),
				m_rclManager(manager),
				m_pclItem(item),
				m_pfnSerializeDeltaProc(serializeDeltaProc)
			{
				//empty
			}

		public:
			const EventType m_kType;

			const Service &m_rclManager;

			const IItem *m_pclItem;

			const SerializeDeltaProc_t m_pfnSerializeDeltaProc;
	};

	class IObjectManagerListener
	{
		public:
			virtual void OnObjectManagerEvent(const ObjectManagerEvent &event) = 0;

			virtual ~IObjectManagerListener()
			{
				//empty
			}
	};

	class Service: public dcclite::FolderObject
	{
		public:
			virtual void Initialize() {};			

			virtual ~Service() {}						

			mutable sigslot::signal< const ObjectManagerEvent &> m_sigEvent;			
	
		protected:
			Service(std::string name, Broker &broker, const rapidjson::Value &params, const Project &project):
				FolderObject(std::move(name)),			
				m_rclBroker(broker),
				m_rclProject(project)
			{
				//empty
			}

			Service(std::string name, Broker& broker, const Project& project) :
				FolderObject(std::move(name)),
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

			void NotifyItemChanged(const dcclite::IItem &item, ObjectManagerEvent::SerializeDeltaProc_t proc = nullptr) const;

			void NotifyItemCreated(const dcclite::IItem &item) const;
			void NotifyItemDestroyed(const dcclite::IItem &item) const;

		private:
			void DispatchEvent(const ObjectManagerEvent &event) const;

		protected:		
			Broker &m_rclBroker;

			const Project &m_rclProject;			
	};
}

