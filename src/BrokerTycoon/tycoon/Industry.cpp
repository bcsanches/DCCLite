// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Industry.h"

#include "TycoonService.h"

#include <dcclite/RName.h>
#include <dcclite/JsonUtils.h>

namespace dcclite::broker::tycoon
{
	CargoHolder::CargoHolder(TycoonService &tycoon, const rapidjson::Value &params) :
		CargoHolder{
			tycoon.FindCargoByName(RName::Get(json::GetString(params, "cargo", "[CargoHolder]"))),
			json::GetFloat(params, "dailyProduction", "[CargoHolder]"),
			static_cast<uint8_t>(json::GetInt(params, "maximumStorage", "[CargoHolder]"))			
		}		
	{
		auto destinationsValue = json::TryGetValue(params, "destinations");
		if (destinationsValue)
		{
			if(!destinationsValue->IsArray())
			{
				throw std::invalid_argument("[CargoHolder::CargoHolder] destinations must be an array");
			}

			for(const auto &it : destinationsValue->GetArray())
			{
				if(!it.IsString())
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] each destination must be a string");
				}

				m_vecDestinations.emplace_back(it.GetString(), it.GetStringLength());
			}
		}

		auto destinationValue = json::TryGetString(params, "destination");
		if(destinationValue)
		{
			m_vecDestinations.emplace_back(*destinationValue);
		}

		//
		//Config production
		auto &clock = tycoon.GetFastClock();		

		m_fProducing = true;
		m_tNextProduction = clock.Now();
	}

	void CargoHolder::Update(const FastClock &fastClock)
	{
		if (!m_fProducing)
			return;

		auto now = fastClock.Now();
		while (now >= m_tNextProduction)
		{
			++m_uCurrentQuantity;

			if(m_uCurrentQuantity == m_uCurrentQuantity)
			{
				//reached max capacity, stop production
				m_fProducing = false;

				return;
			}
			
			this->ScheduleProduction(fastClock);
		}
	}

	void CargoHolder::Consume(const FastClock &fastClock)
	{		
		if (m_uCurrentQuantity == 0)
		{
			throw std::runtime_error("[CargoHolder::Consume] No cargo available to consume");
		}

		--m_uCurrentQuantity;

		//only schedule production if we are not already producing
		if(!m_fProducing)
		{
			this->ScheduleProduction(fastClock);
		}
	}

	void CargoHolder::ScheduleProduction(const FastClock &fastClock)
	{
		m_fProducing = true;		

		auto now = fastClock.Now();

		constexpr auto secondsPerDay = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{ 24 });
		auto interval = std::chrono::seconds{ static_cast<long>(secondsPerDay.count() / m_fDailyRate) };

		m_tNextProduction = now + interval;
	}

	const char *Industry::TYPE_NAME = "dcclite::broker::tycoon::Industry";

	Industry::Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params):
		Object{name},
		m_clCargoHolder{tycoon, params["produce"]}
	{
		//empty
	}
}
