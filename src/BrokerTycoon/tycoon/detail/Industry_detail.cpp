// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Industry_detail.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <magic_enum/magic_enum.hpp>

#include <dcclite/AccessToken.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>

#include "../Cargo.h"
#include "../FastClock.h"
#include "../FastClockUtils.h"
#include "../Industry.h"
#include "../TycoonService.h"

namespace dcclite::broker::tycoon::detail
{
	///////////////////////////////////////////////////////////////////////////
	//
	//
	// Spot
	//
	//
	///////////////////////////////////////////////////////////////////////////
	void Spot::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddStringValue("name", this->GetNameData());
		stream.AddStringValue("state", magic_enum::enum_name(m_kState));
		stream.AddStringValue("info", m_strInformation);
	}

	void Spot::Load(int cargoIndex)
	{
		if (!this->CanLoad())
		{
			throw std::runtime_error("[Spot::Load] Spot is not reserved to load");
		}

		m_kState = SpotStates::LOADING;
		m_iCargoIndex = cargoIndex;
	}

	///////////////////////////////////////////////////////////////////////////
	//
	//
	// CargoInfo
	//
	//
	///////////////////////////////////////////////////////////////////////////
	CargoInfo::CargoInfo(const TycoonService &tycoon, const rapidjson::Value &params):
		m_rclCargo{ tycoon.FindCargoByName(RName::Get(json::GetString(params, "cargo", "[Tycoon::detail::CargoInfo]"))) },
		m_tTransferTime{ std::chrono::hours{ json::GetInt(params, "transferTimeHours", "[CargoHolder]") } }
	{
		if (m_tTransferTime <= std::chrono::hours{ 0 })
		{
			throw std::invalid_argument("[CargoHolder::CargoInfo] transferTimeHours must be greater than zero");
		}

		auto chance = json::TryGetDefaultInt(params, "chance", 1);
		if(chance < 1 || chance > 255)
		{
			throw std::invalid_argument("[CargoHolder::CargoInfo] chance must be between 1 and 255");
		}

		m_u8Chance = static_cast<uint8_t>(chance);
		this->LoadDestinations(params);
	}

	void CargoInfo::LoadDestinations(const rapidjson::Value &params)
	{
		auto destinationsValue = json::TryGetValue(params, "destinations");
		if (destinationsValue)
		{
			if (!destinationsValue->IsArray())
			{
				throw std::invalid_argument("[Tycoon::CargoInfo::LoadDestinations] destinations must be an array");
			}

			auto destionationsArray = destinationsValue->GetArray();
			m_vecDestinations.reserve(destionationsArray.Size());

			for (const auto &it : destionationsArray)
			{
				if (!it.IsString())
				{
					throw std::invalid_argument("[Tycoon::CargoInfo::LoadDestinations] each destination must be a string");
				}

				m_vecDestinations.emplace_back(it.GetString(), it.GetStringLength());
			}
		}

		auto destinationValue = json::TryGetString(params, "destination");
		if (destinationValue)
		{
			m_vecDestinations.emplace_back(*destinationValue);
		}

		if (m_vecDestinations.empty())
		{
			throw std::invalid_argument("[Tycoon::CargoInfo::LoadDestinations] at least one destination must be specified");
		}
	}

	void CargoInfo::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddStringValue("cargo", this->m_rclCargo.GetNameData());
		stream.AddIntValue("transferTimeHours", static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(m_tTransferTime).count()));
		stream.AddIntValue("chance", m_u8Chance);

		{
			auto destionationsArray = stream.AddArray("destinations");

			for (auto &d : m_vecDestinations)
			{
				destionationsArray.AddString(d);
			}
		}

		this->SerializeDelta(stream);
	}

	void CargoInfo::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddIntValue("currentQuantity", m_uCurrentQuantity);
		stream.AddIntValue("reservedQuantity", m_uReservedQuantity);
	}

	std::chrono::hours CargoInfo::StartCargoTransfer()
	{
		if (m_uCurrentQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoInfo::StartCargoTransfer] No cargo in stock!!!");
		}

		--m_uCurrentQuantity;
		++m_uReservedQuantity;

		return m_tTransferTime;
	}

	void CargoInfo::CompleteCargoTransfer()
	{
		if (m_uReservedQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoInfo::CompleteCargoTransfer] No cargo reserved!!!");
		}

		--m_uReservedQuantity;		
	}

	///////////////////////////////////////////////////////////////////////////
	//
	//
	// CargoHolder
	//
	//
	///////////////////////////////////////////////////////////////////////////
	CargoHolder::CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity, std::chrono::hours transferTime, FastClock &fastClock, const Industry &industry):
		m_clProductionThinker{ fastClock.MakeThinker("CargoHolder::ProductionThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker)) },
		m_rclFastClock{ fastClock },
		m_rclIndustry{ industry },
		m_rclCargo{ cargo },
		m_fpDailyRate{ dailyRate },
		m_uMaxQuantity{ maxQuantity },
		m_tTransferTime{ transferTime },
		m_fProducing{ true }
	{
		if (dailyRate <= 0.0f)
		{
			throw std::invalid_argument("[CargoHolder::CargoHolder] dailyRate must be greater than zero");
		}

		if (maxQuantity == 0)
		{
			throw std::invalid_argument("[CargoHolder::CargoHolder] maxQuantity must be greater than zero");
		}

		if (m_tTransferTime <= std::chrono::hours{ 0 })
		{
			throw std::invalid_argument("[CargoHolder::CargoHolder] transferTimeHours must be greater than zero");
		}
	}

	CargoHolder::CargoHolder(TycoonService &tycoon, const Industry &industry, const rapidjson::Value &params) :
		CargoHolder{
			tycoon.FindCargoByName(RName::Get(json::GetString(params, "cargo", "[Tycoon::CargoHolder]"))),
			json::GetFloat(params, "dailyProduction", "[Tycoon::CargoHolder]"),
			static_cast<uint8_t>(json::GetInt(params, "maximumStorage", "[CargoHolder]")),
			std::chrono::hours{json::GetInt(params, "transferTimeHours", "[CargoHolder]")},
			tycoon.GetFastClock(),
			industry
	}
	{
		auto destinationsValue = json::TryGetValue(params, "destinations");
		if (destinationsValue)
		{
			if (!destinationsValue->IsArray())
			{
				throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] destinations must be an array");
			}

			for (const auto &it : destinationsValue->GetArray())
			{
				if (!it.IsString())
				{
					throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] each destination must be a string");
				}

				m_vecDestinations.emplace_back(it.GetString(), it.GetStringLength());
			}
		}

		auto destinationValue = json::TryGetString(params, "destination");
		if (destinationValue)
		{
			m_vecDestinations.emplace_back(*destinationValue);
		}

		if (m_vecDestinations.empty())
		{
			throw std::invalid_argument("[Tycoon::CargoHolder::CargoHolder] at least one destination must be specified");
		}

		//
		//Config production
		this->ScheduleProduction();
	}

	std::chrono::hours CargoHolder::StartCargoTransfer()
	{
		if (m_uCurrentQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoHolder::StartCargoTransfer] No cargo in stock!!!");
		}

		--m_uCurrentQuantity;
		++m_uReservedQuantity;

		return m_tTransferTime;
	}

	void CargoHolder::CargoTransferFinished()
	{
		if (m_uReservedQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoHolder::Consume] No cargo reserved!!!");
		}

		--m_uReservedQuantity;

		//only schedule production if we are not already producing
		if (!m_fProducing)
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

		if (!this->CanProduce())
		{
			dcclite::Log::Trace("[Tycoon::CargoHolder::ScheduleProduction] Stock is full of {}, halted production", m_rclCargo.GetName());

			//reached max capacity, stop production
			m_fProducing = false;
		}
		else
		{
			this->ScheduleProduction();
		}

		m_rclIndustry.OnCargoHolderStateChanged(AccessToken<CargoHolder>{});
	}

	void CargoHolder::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddIntValue("currentQuantity", m_uCurrentQuantity);
		stream.AddIntValue("reservedQuantity", m_uReservedQuantity);
		stream.AddBool("producing", m_fProducing);

		if (m_fProducing)
		{
			auto nextProductionTime = m_clProductionThinker.GetTimePoint();

			auto localTime = FastClockUtils::GetLocalTime(nextProductionTime, m_rclFastClock);

			stream.AddStringValue(
				"nextProductionAtLocalTime",
				fmt::format("{:%H:%M}", localTime)
			);

			stream.AddStringValue(
				"nextProductionAt",
				fmt::format("{:%H:%M}", nextProductionTime.time_since_epoch())
			);
		}
		else
		{
			stream.AddStringValue("nextProductionAt", "N/A");
			stream.AddStringValue("nextProductionAtLocalTime", "N/A");
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
}
