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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <magic_enum/magic_enum.hpp>

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>

#include "sys/Project.h"
#include "sys/ServiceFactory.h"

#include "FastClock.h"

#include "Cargo.h"

namespace dcclite::broker::tycoon
{
	class TycoonServiceImpl : public TycoonService
	{
		public:
			TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params);

		private:
			FastClock			m_clFastClock;

			std::vector<Cargo>	m_vecCargos;

			dcclite::fs::path	m_pathDataFileName;
	};

	class JsonFileDocument
	{
		public:
			[[nodiscard]] bool Load(const dcclite::fs::path &path)
			{
				std::ifstream configFile(path);

				if (!configFile)
				{
					dcclite::Log::Warn("[JsonFileDocument::Load] cannot find {}", path.string());

					return false;
				}

				rapidjson::IStreamWrapper isw(configFile);
				
				m_docJson.ParseStream(isw);
				if(m_docJson.HasParseError())
				{
					throw std::runtime_error(
						fmt::format(
							"[JsonFileDocument::Load] error parsing JSON file {}: {}", 
							path.string(), 
							magic_enum::enum_name(m_docJson.GetParseError())
						)
					);
				}

				return true;
			}

			[[nodiscard]] inline bool IsObject() const noexcept
			{
				return m_docJson.IsObject();
			}			

			[[nodiscard]] const rapidjson::Value &GetObject() const noexcept
			{
				return m_docJson.GetObject();
			}

		private:			
			rapidjson::Document m_docJson;
	};

	TycoonServiceImpl::TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		TycoonService(name, broker, params),
		m_clFastClock{ RName{"FastClock"}, 1 },
		m_pathDataFileName{ sys::Project::GetFilePath(name.GetData()) }
	{
		BenchmarkLogger benchmark{ "TycoonServiceImpl::TycoonServiceImpl", this->GetNameData() };
		m_pathDataFileName.concat(".config.json");

		dcclite::Log::Info("[TycoonServiceImpl::{}] [Load] Trying to load {}", this->GetName(), m_pathDataFileName.string());

		const rapidjson::Value *configData = &params;

		JsonFileDocument fileDocument;
		if (fileDocument.Load(m_pathDataFileName))
		{
			if (!fileDocument.IsObject())
				throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid config, expected object definition inside config", this->GetName()));

			dcclite::Log::Info("[TycoonServiceImpl::{}] [Load] Loaded {}, using it instead of inline definition", this->GetName(), m_pathDataFileName.string());
			configData = &fileDocument.GetObject();
		}			
		
		auto clockRate = dcclite::json::TryGetDefaultInt(*configData, "fastClockRate", 4);
		if(clockRate <= 0)
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be greater than zero", this->GetName()));
		}

		if (clockRate > std::numeric_limits<uint8_t>::max())
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be less or equal than {}", this->GetName(), std::numeric_limits<uint8_t>::max()));
		}

		m_clFastClock.SetRate(clockRate);

		auto cargosArray = dcclite::json::TryGetValue(*configData, "cargos");
		if (cargosArray)
		{
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
				auto it = std::ranges::find_if(m_vecCargos, [rname](const Cargo &c) { return c.GetName() == rname; });
				if(it != m_vecCargos.end())
				{
					throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: duplicate cargo name '{}'", this->GetName(), name.value()));
				}

				m_vecCargos.emplace_back(rname);
			}
		}

		m_clFastClock.Start();
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

	TycoonService::TycoonService(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		Service(name, broker, params)		
	{
		//empty
	}

	static sys::GenericServiceFactory<TycoonServiceImpl> g_ServiceFactory;
}
