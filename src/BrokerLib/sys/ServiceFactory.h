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

#include <JsonUtils.h>
#include <RName.h>

#include "Broker.h"

namespace dcclite::broker
{
	class Service;	

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

			virtual std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data) const = 0;

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

			inline std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data) const override
			{
				return std::make_unique<T>(name, broker, data);
			}
	};

	template <typename T>
	class GenericServiceWithDependenciesFactory: public ServiceFactory
	{
		public:
			GenericServiceWithDependenciesFactory(): ServiceFactory(RName{ T::TYPE_NAME })
			{
				this->Register();
			}

			inline std::unique_ptr<Service> Create(RName name, Broker &broker, const rapidjson::Value &data) const override
			{
				return std::make_unique<T>(name, broker, data, this->ResolveRequirement(broker, data));
			}

		private:
			T::Requirement_t &ResolveRequirement(const Broker &broker, const rapidjson::Value &data) const
			{
				auto requirementId = dcclite::json::TryGetDefaultString(
					data,
					"requires",
					T::Requirement_t::TYPE_NAME
				);

				return dynamic_cast<T::Requirement_t &>(broker.ResolveRequirement(requirementId));
			}
	};
}

