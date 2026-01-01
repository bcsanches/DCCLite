// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TycoonService.h"

#include <fstream>

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>

#include "sys/Project.h"
#include "sys/ServiceFactory.h"

#include "Cargo.h"
#include "CarType.h"
#include "Location.h"

namespace dcclite::broker::tycoon
{
	TycoonService::TycoonService(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		Service(name, broker, params),
		m_clFastClock{ RName{"FastClock"}, 1 },
		m_pathDataFileName{ sys::Project::GetFilePath(name.GetData()) }
	{
		BenchmarkLogger benchmark{ "TycoonServiceImpl::TycoonServiceImpl", this->GetNameData() };

		m_pLocations = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "locations" })));

		m_pathDataFileName.concat(".config.json");

		dcclite::Log::Info("[TycoonServiceImpl::{}] [Load] Trying to load {}", this->GetName(), m_pathDataFileName.string());		

		dcclite::json::FileDocument fileDocument;
		if (fileDocument.Load(m_pathDataFileName))
		{
			if (!fileDocument.IsObject())
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid config, expected object definition inside config", this->GetName()));

			dcclite::Log::Info("[TycoonServiceImpl::{}] [Load] Loaded {}, using it instead of inline definition", this->GetName(), m_pathDataFileName.string());
			
			this->Load(fileDocument.GetObject());
		}
		else
		{
			this->Load(params);
		}
				
		m_clFastClock.Start();
	}

	const Cargo *TycoonService::TryFindCargoByName(RName name) const noexcept
	{
		auto it = std::ranges::find_if(m_vecCargos, [name](const Cargo &c) { return c.GetName() == name; });
		if (it != m_vecCargos.end())
		{
			return &*it;
		}

		return nullptr;
	}

	const Cargo &TycoonService::FindCargoByName(RName name) const
	{
		auto cargo = this->TryFindCargoByName(name);
		if (!cargo)
		{
			throw std::invalid_argument(
				fmt::format(
					"[TycoonServiceImpl::{}] [FindCargoByName] error: unknown cargo name '{}'",
					this->GetName(),
					name
				)
			);
		}

		return *cargo;
	}

	void TycoonService::LoadCargos(const rapidjson::Value &params)
	{
		auto cargosArray = dcclite::json::TryGetValue(params, "cargos");
		if (!cargosArray)
			return;

		if (!cargosArray->IsArray())
		{
			throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid cargos definition, expected array", this->GetName()));
		}

		for (const auto &cargoValue : cargosArray->GetArray())
		{
			if (!cargoValue.IsObject())
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid cargo definition, expected object", this->GetName()));
			}

			auto name = dcclite::json::TryGetString(cargoValue, "name");
			if (!name)
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: cargo missing name property", this->GetName()));
			}

			RName rname{ name.value() };

			if(this->TryFindCargoByName(rname))
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: duplicate cargo name '{}'", this->GetName(), name.value()));
			}			

			m_vecCargos.emplace_back(rname);
		}	
	}

	void TycoonService::AddCargoToCarType(CarType &carType, std::string_view cargoName)
	{		
		auto rcargoName{ RName::Get(cargoName) };

		const Cargo *cargo = this->TryFindCargoByName(rcargoName);
		if (!cargo)
		{
			throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [AddCargoToCarType] error: carType '{}' references unknown cargo '{}'", this->GetName(), carType.GetName(), cargoName));
		}

		carType.AddCargo(*cargo);
	}

	void TycoonService::LoadCarTypes(const rapidjson::Value &params)
	{
		auto carTypesArray = dcclite::json::TryGetValue(params, "carTypes");
		if (!carTypesArray)
			return;

		if (!carTypesArray->IsArray())
		{
			throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid carTypes definition, expected array", this->GetName()));
		}

		for (const auto &carTypeValue : carTypesArray->GetArray())
		{
			if (!carTypeValue.IsObject())
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid carType definition, expected object", this->GetName()));
			}

			auto name = dcclite::json::TryGetString(carTypeValue, "name");
			if (!name)
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: carType missing name property", this->GetName()));
			}

			RName rname{ name.value() };
			auto it = std::ranges::find_if(m_vecCarTypes, [rname](const CarType &c) { return c.GetName() == rname; });
			if (it != m_vecCarTypes.end())
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: duplicate carType name '{}'", this->GetName(), name.value()));
			}

			auto carType = dcclite::json::TryGetString(carTypeValue, "ABNT");
			if (!carType)
			{
				carType = dcclite::json::TryGetString(carTypeValue, "AAR");
				if(!carType)
					throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: carType '{}' missing type property, must be ABNT or AAR", this->GetName(), name.value()));
			}
			RName rcarType{ carType.value() };

			std::string description{ dcclite::json::TryGetDefaultString(carTypeValue, "description", "") };

			m_vecCarTypes.emplace_back(rname, rcarType, std::move(description));
			auto &newCarType = m_vecCarTypes.back();
			
			try
			{
				if (auto cargoName = dcclite::json::TryGetString(carTypeValue, "cargo"))
				{
					this->AddCargoToCarType(newCarType, cargoName.value());
				}

				auto cargosArray = dcclite::json::TryGetValue(carTypeValue, "cargos");
				if (cargosArray)
				{
					if (!cargosArray->IsArray())
						throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: carType '{}' has invalid cargos definition, expected array", this->GetName(), name.value()));

					for (const auto &cargoValue : cargosArray->GetArray())
					{
						this->AddCargoToCarType(newCarType, cargoValue.GetString());
					}
				}

				if (!newCarType.HasAnyCargo())
				{
					throw std::invalid_argument(
						fmt::format(
							"[TycoonServiceImpl::{}] [Load] error: carType '{}' has no cargos assigned",
							this->GetName(),
							newCarType.GetName()
						)
					);
				}
			}	
			catch(...)
			{
				//remove the newly created car type on error
				m_vecCarTypes.pop_back();

				throw;
			}
		}
	}

	void TycoonService::LoadLocations(const rapidjson::Value &params)
	{
		auto locationsArray = dcclite::json::TryGetValue(params, "locations");
		if (!locationsArray)
			return;

		if (!locationsArray->IsArray())
		{
			throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [LoadLocations] error: invalid locations definition, expected array", this->GetName()));
		}

		for (const auto &locationValue : locationsArray->GetArray())
		{
			if (!locationValue.IsObject())
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [LoadLocations] error: location invalid definition, expected object", this->GetName()));
			}
			
			auto name = dcclite::json::TryGetString(locationValue, "name");
			if (!name)
			{
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [LoadLocations] error: location missing name property", this->GetName()));
			}

			m_pLocations->AddChild(std::make_unique<Location>(RName{ name.value() }, *this, locationValue));
		}
	}

	void TycoonService::Load(const rapidjson::Value &params)
	{		
		auto clockRate = dcclite::json::TryGetDefaultInt(params, "fastClockRate", 4);
		if (clockRate <= 0)
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be greater than zero", this->GetName()));
		}

		if (clockRate > std::numeric_limits<uint8_t>::max())
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be less or equal than {}", this->GetName(), std::numeric_limits<uint8_t>::max()));
		}

		m_clFastClock.SetRate(clockRate);

		this->LoadCargos(params);
		this->LoadCarTypes(params);
		this->LoadLocations(params);
	}

	//
	//
	// TycoonServiceFactory
	//
	//

	const char *TycoonService::TYPE_NAME = "TycoonService";

	void TycoonService::RegisterFactory()
	{
		//empty
	}

	static sys::GenericServiceFactory<TycoonService> g_ServiceFactory;
}
