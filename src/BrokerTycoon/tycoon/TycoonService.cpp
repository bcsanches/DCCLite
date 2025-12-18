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

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>

#include "sys/Project.h"
#include "sys/ServiceFactory.h"

#include "FastClock.h"

namespace dcclite::broker::tycoon
{

	class TycoonServiceImpl : public TycoonService
	{
		public:
			TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params);

		private:
			FastClock m_clFastClock;

			dcclite::fs::path m_pathDataFileName;
	};

	TycoonServiceImpl::TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		TycoonService(name, broker, params),
		m_clFastClock{ RName{"FastClock"}, 1 },
		m_pathDataFileName{ sys::Project::GetFilePath(name.GetData()) }
	{
		BenchmarkLogger benchmark{ "TycoonServiceImpl::TycoonServiceImpl", this->GetNameData() };
		m_pathDataFileName.concat(".config.json");

		dcclite::Log::Info("[TycoonServiceImpl::{}] [Load] Loading {}", this->GetName(), m_pathDataFileName.string());
		std::ifstream configFile(m_pathDataFileName);
		if (!configFile)
		{
			dcclite::Log::Error("[TycoonServiceImpl::{} [Load] cannot find {}", this->GetName(), m_pathDataFileName.string());

			return;
		}

		rapidjson::IStreamWrapper isw(configFile);
		rapidjson::Document decodersData;
		decodersData.ParseStream(isw);

		if (!decodersData.IsObject())
			throw std::runtime_error(fmt::format("[TycoonServiceImpl::{}] [Load] error: invalid config, expected object definition inside config", this->GetName()));

		auto clockRate = dcclite::json::TryGetDefaultInt(params, "fastClockRate", 4);
		if(clockRate <= 0)
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be greater than zero", this->GetName()));
		}

		if (clockRate > std::numeric_limits<uint8_t>::max())
		{
			throw std::invalid_argument(fmt::format("[TycoonServiceImpl::{}] [Load] fastClockRate must be less than {}", this->GetName(), std::numeric_limits<uint8_t>::max()));
		}

		m_clFastClock.SetRate(clockRate);

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
