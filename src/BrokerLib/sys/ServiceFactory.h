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
#include <memory>

#include <rapidjson/document.h>

#include <RName.h>

#define DCC_LITE_SERVICE_FACTORY_REQ(NAME, CLASS_NAME, REQUIREMENTS, CODE)			 \
		static std::unique_ptr<dcclite::broker::Service> PROC_##NAME(dcclite::RName name, dcclite::broker::Broker &broker, const rapidjson::Value &data, const dcclite::broker::Project &project, Service &dep) { CODE }	\
		static dcclite::broker::ServiceWithDependenciesFactory NAME{CLASS_NAME, REQUIREMENTS, PROC_##NAME};

#define DCC_LITE_SERVICE_FACTORY(NAME, CLASS_NAME, CODE)  \
		static std::unique_ptr<dcclite::broker::Service> PROC_##NAME(dcclite::RName name, dcclite::broker::Broker &broker, const rapidjson::Value &data, const dcclite::broker::Project &project) { CODE }	\
		static dcclite::broker::DefaultServiceFactory NAME{CLASS_NAME, PROC_##NAME};

namespace dcclite::broker
{
	class Service;
	class Broker;
	class Project;

	class ServiceFactory
	{
		public:
			static void RegisterAll();
			static ServiceFactory *TryFindFactory(RName name) noexcept;

		protected:
			ServiceFactory(RName className);

			void Register();

		public:

			virtual ~ServiceFactory() = default;

			virtual std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data, const Project &project) const = 0;

			virtual bool HasDependencies() const noexcept
			{
				return false;
			}			

		private:
			RName m_clClassName;

			ServiceFactory *m_pclNext = nullptr;
	};

	template <typename T>
	class GenericServiceFactory: public ServiceFactory
	{
		public:
			GenericServiceFactory(): ServiceFactory(RName{ T::TYPE_NAME })
			{
				this->Register();
			}

			inline std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data, const Project &project) const override
			{
				return std::make_unique<T>(name, broker, data, project);
			}
	};

	class DefaultServiceFactory: public ServiceFactory
	{
		public:
			typedef std::unique_ptr<Service>(*FactoryProc_t)(RName name, Broker &broker, const rapidjson::Value &data, const Project &project);
			
			DefaultServiceFactory(RName className, FactoryProc_t proc);

			inline std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data, const Project &project) const override
			{				
				return m_pfnProc(name, broker, data, project);
			}	

		private:
			FactoryProc_t m_pfnProc;			
	};

	class ServiceWithDependenciesFactory: public ServiceFactory
	{
		public:
			typedef std::unique_ptr<Service>(*FactoryProc_t)(RName name, Broker &broker, const rapidjson::Value &data, const Project &project, Service &dependency);

			ServiceWithDependenciesFactory(RName className, const char *defaultRequirement, FactoryProc_t proc);

			std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data, const Project &project) const override;

			virtual bool HasDependencies() const noexcept
			{
				return true;
			}

		private:
			FactoryProc_t m_pfnProc;

			const char *m_pszDefaultRequirement = nullptr;
	};
}

