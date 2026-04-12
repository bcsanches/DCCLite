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

#include <chrono>

#include <fmt/chrono.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/RName.h>
#include <dcclite/JsonUtils.h>

#include "Cargo.h"
#include "FastClockUtils.h"
#include "TycoonService.h"

namespace dcclite::broker::tycoon
{
	CargoHolder::CargoHolder(TycoonService &tycoon, const Industry &industry, const rapidjson::Value &params) :
		CargoHolder{
			tycoon.FindCargoByName(RName::Get(json::GetString(params, "cargo", "[Tycoon::CargoHolder]"))),
			json::GetFloat(params, "dailyProduction", "[Tycoon::CargoHolder]"),
			static_cast<uint8_t>(json::GetInt(params, "maximumStorage", "[CargoHolder]")),
			tycoon,
			tycoon.GetFastClock(),
			industry
		}
	{
		auto destinationsValue = json::TryGetValue(params, "destinations");
		if (destinationsValue)
		{
			if(!destinationsValue->IsArray())
			{
				throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] destinations must be an array");
			}

			for(const auto &it : destinationsValue->GetArray())
			{
				if(!it.IsString())
				{
					throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] each destination must be a string");
				}

				m_vecDestinations.emplace_back(it.GetString(), it.GetStringLength());
			}
		}

		auto destinationValue = json::TryGetString(params, "destination");
		if(destinationValue)
		{
			m_vecDestinations.emplace_back(*destinationValue);
		}

		if(m_vecDestinations.empty())
		{
			throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] at least one destination must be specified");
		}

		//
		//Config production
		this->ScheduleProduction();
	}

	void CargoHolder::Consume(const FastClock &fastClock)
	{		
		if (m_uCurrentQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoHolder::Consume] No cargo available to consume");
		}

		--m_uCurrentQuantity;

		//only schedule production if we are not already producing
		if(!m_fProducing)
		{
			this->ScheduleProduction();
		}
	}

	void CargoHolder::ScheduleProduction()
	{
		m_fProducing = true;		

		auto now = m_rclFastClock.Now();

		constexpr auto secondsPerDay = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{ 24 });
		auto interval = std::chrono::seconds{ static_cast<long>(secondsPerDay.count() / m_fpDailyRate) };

		m_clProductionThinker.Schedule(now + interval);

		auto realTimeInverval = m_rclFastClock.ConvertToRealTime(interval);
		dcclite::Log::Trace("[Tycoon::CargoHolder::ScheduleProduction] Next production of {} in ~{} (~{})", m_rclCargo.GetName(), realTimeInverval, std::chrono::duration_cast<std::chrono::minutes>(realTimeInverval));
	}

	void CargoHolder::ProduceThinker(FastClockDef::TimePoint_t tp)
	{
		++m_uCurrentQuantity;

		dcclite::Log::Trace("[Tycoon::CargoHolder::ScheduleProduction] Produced {}, now we have {}", m_rclCargo.GetName(), m_uCurrentQuantity);

		if(m_uCurrentQuantity == m_uMaxQuantity)
		{
			dcclite::Log::Trace("[Tycoon::CargoHolder::ScheduleProduction] Stock is full of {}, halted production", m_rclCargo.GetName());

			//reached max capacity, stop production
			m_fProducing = false;			
		}
		else
		{
			this->ScheduleProduction();
		}

		m_rclTycoon.OnObjectStateChanged(IndustryToken{}, m_rclIndustry);
	}

	void CargoHolder::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{		
		stream.AddIntValue("currentQuantity", m_uCurrentQuantity);
		stream.AddBool("producing", m_fProducing);

		if (m_fProducing)
		{
			auto nextProductionTime = m_clProductionThinker.GetTimePoint();

			auto localTime = FastClockUtils::GetLocalTime(nextProductionTime, m_rclFastClock);			
			
			stream.AddStringValue(
				"nextProductionAt",
				fmt::format("{:%H:%M}", localTime)
			);
		}
		else
		{
			stream.AddStringValue("nextProductionAt", "N/A");
		}
	}

	void CargoHolder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		{
			auto destionationsArray = stream.AddArray("destinations");

			for (auto &d : m_vecDestinations)
			{
				destionationsArray.AddString(d);
			}
		}

		stream.AddStringValue("cargo", m_rclCargo.GetNameData());
		stream.AddFloatValue("dailyRate", m_fpDailyRate);
		stream.AddIntValue("maximumQuantity", m_uMaxQuantity);

		this->SerializeDelta(stream);
	}

	const char *Industry::TYPE_NAME = "dcclite::broker::tycoon::Industry";

	Industry::Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params):
		Object{name},
		m_clCargoHolder{tycoon, *this, params["produce"]}
	{		
		if(auto spot = json::TryGetString(params, "spot"))
		{
			m_vecSpots.emplace_back(RName{ *spot });
		}

		auto spotsValue = params.FindMember("spots");
		if(spotsValue == params.MemberEnd())
		{
			if(m_vecSpots.empty())
			{
				throw std::invalid_argument("[Industry::Industry] either spot or spots must be specified");
			}

			return;
		}

		if (!m_vecSpots.empty())
		{
			throw std::invalid_argument("[Industry::Industry] spot and spots cannot be specified at the same time");
		}

		for(auto &it : spotsValue->value.GetArray())
		{
			if(!it.IsString())
			{
				throw std::invalid_argument("[Industry::Industry] each spot must be a string");
			}
			m_vecSpots.emplace_back(RName{ it.GetString() });
		}

		if(m_vecSpots.empty())
		{
			throw std::invalid_argument("[Industry::Industry] at least one spot must be specified");
		}
	}

	void Industry::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Object::Serialize(stream);

		m_clCargoHolder.Serialize(stream);
		
		auto spotsData = stream.AddArray("spots");
		for(auto &spot : m_vecSpots)
		{
			spotsData.AddString(spot.GetNameData());
		}
	}

	void Industry::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{
		this->SerializeIdentification(stream);

		m_clCargoHolder.SerializeDelta(stream);
	}
}
