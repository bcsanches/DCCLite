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

#include <chrono>

#include <fmt/chrono.h>

#include <dcclite/Benchmark.h>
#include <dcclite/FileSystem.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>

#include "sys/Project.h"
#include "sys/ServiceFactory.h"

#include "Cargo.h"
#include "CarType.h"
#include "Industry.h"
#include "Location.h"

namespace dcclite::broker::tycoon
{
	TycoonService::TycoonService(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		Service(name, broker, params),
		m_clFastClock{ 1 },
		m_pathDataFileName{ sys::Project::GetFilePath(name.GetData()) }
	{
		BenchmarkLogger benchmark{ "TycoonService::TycoonService", this->GetNameData() };

		m_pLocations = static_cast<FolderObject *>(this->AddChild(std::make_unique<FolderObject>(RName{ "locations" })));

		m_pathDataFileName.concat(".config.json");

		dcclite::Log::Info("[TycoonService::{}] [Load] Trying to load {}", this->GetName(), m_pathDataFileName.string());		

		dcclite::json::FileDocument fileDocument;
		if (fileDocument.Load(m_pathDataFileName))
		{
			if (!fileDocument.IsObject())
				throw std::runtime_error(fmt::format("[TycoonService::{}] [Load] error: invalid config, expected object definition inside config", this->GetName()));

			dcclite::Log::Info("[TycoonService::{}] [Load] Loaded {}, using it instead of inline definition", this->GetName(), m_pathDataFileName.string());
			
			this->Load(fileDocument.GetObject());
		}
		else
		{
			this->Load(params);
		}
				
		m_clFastClock.m_sigTick.connect([this](FastClock &clock) { this->OnFastClockTick(clock); });
		m_clFastClock.Start();

		this->LoadState();
	}

	TycoonService::~TycoonService()
	{
		this->SaveState();
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
					"[TycoonService::{}] [FindCargoByName] error: unknown cargo name '{}'",
					this->GetName(),
					name
				)
			);
		}

		return *cargo;
	}	

	void TycoonService::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		sys::Service::Serialize(stream);

		if (!m_vecCargos.empty())
		{
			auto cargosData = stream.AddArray("cargos");

			for (const auto &c : m_vecCargos)
			{
				cargosData.AddString(c.GetNameData());
			}
		}

		if (!m_vecCarTypes.empty())
		{
			auto carTypesData = stream.AddArray("carTypes");

			for (const auto &c : m_vecCarTypes)
			{
				auto carTypeData = carTypesData.AddObject();
				c.Serialize(carTypeData);
			}
		}
	}
	
	void TycoonService::OnObjectStateChanged(AccessToken<Industry>, const Industry &industry, dcclite::broker::sys::ObjectManagerEvent::SerializeDeltaProc_t proc)
	{
		this->NotifyItemChanged(
			industry, 
			proc ? proc : [&industry](JsonOutputStream_t &stream) { industry.SerializeDelta(stream); }
		);
	}

	void TycoonService::OnFastClockTick(FastClock &clock)
	{
		this->NotifyItemChanged(*this, [this](JsonOutputStream_t &stream) 
			{
				this->SerializeIdentification(stream);

				auto str = fmt::format("{:%H:%M}", m_clFastClock.Now().time_since_epoch());
				stream.AddStringValue("fast_clock_time", str);
			}
		);
	}

	//
	//
	// State Storage
	//
	//

	static dcclite::fs::path GenerateBaseStateFileName(RName serviceName)
	{
		return sys::Project::GetAppFilePath(fmt::format("{}.state", serviceName.GetData()));
	}

	void TycoonService::SaveState()
	{
		JsonCreator::StringWriter responseWriter;
		{
			auto object = JsonCreator::MakeObject(responseWriter);

			{
				auto fastClock = object.AddObject("fastClock");
				m_clFastClock.SaveState(fastClock);
			}

			auto locations = object.AddObject("locations");

			//we only save industry state, because cargo and car types are static data, so they will be loaded from config file on next load
			m_pLocations->VisitChildren([&locations](IObject &obj)
				{
					const auto &location = dynamic_cast<Location &>(obj);					

					auto locationStream = locations.AddObject(location.GetNameData());

					location.SaveState(locationStream);
					return true;
				}
			);
			
		}

		auto stateFileName = GenerateBaseStateFileName(this->GetName());

		dcclite::Log::Info("[TycoonService::SaveState][{}] Generating state file: {}", this->GetName(), stateFileName.string());

		if (!FileSystem::SafeStoreText(stateFileName, ".json", responseWriter.GetString()))
		{
			dcclite::Log::Error("[TycoonService::SaveState][{}] Error storing device data at {}", this->GetName(), stateFileName.string());

			return;
		}

		dcclite::Log::Info("[TycoonService::SaveState][{}] State file stored.", this->GetName());
	}

	void TycoonService::LoadState()
	{
		auto stateFileName = GenerateBaseStateFileName(this->GetName());
		stateFileName.concat(".json");

		dcclite::json::FileDocument document;

		if (!document.Load(stateFileName))
		{
			dcclite::Log::Warn("[TycoonService::LoadState] [{}] Failed to open state file: {}", this->GetName(), stateFileName.string());

			return;
		}

		dcclite::Log::Info("[TycoonService::LoadState] [{}] Opened {}, starting parser", this->GetName(), stateFileName.string());

		if (!document.IsObject())
		{
			dcclite::Log::Error("[TycoonService::LoadState] [{}] Expected json object on file {}", this->GetName(), stateFileName.string());

			return;
		}

		auto data = document.GetObject();

		//load the data...
		auto fastClockData = dcclite::json::TryGetObject(data, "fastClock");
		if (fastClockData == nullptr)
		{
			dcclite::Log::Error("[TycoonService::LoadState] [{}] Missing 'fastClock' object in state file {}, cannot load state", this->GetName(), stateFileName.string());

			return;
		}

		m_clFastClock.LoadState(*fastClockData);

		auto locationsData = dcclite::json::TryGetObject(data, "locations");
		if (!locationsData)
		{
			dcclite::Log::Error("[TycoonService::LoadState] [{}] Missing 'locations' object in state file {}, cannot load state", this->GetName(), stateFileName.string());
			return;
		}
			
		for (const auto &locationData : locationsData->GetObject())
		{
			RName locationName = RName::TryGetName(locationData.name.GetString());
			if (!locationName)
			{
				dcclite::Log::Warn("[TycoonService::LoadState] [{}] Unknown location name '{}', skipping", this->GetName(), locationData.name.GetString());
				continue;
			}

			auto obj = m_pLocations->TryGetChild(locationName);
			if (!obj)
			{
				dcclite::Log::Warn("[TycoonService::LoadState] [{}] Location '{}' not found in service, skipping", this->GetName(), locationName.GetData());
				continue;
			}

			auto &location = dynamic_cast<Location &>(*obj);
			location.LoadState(locationData.value);				
		}
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
