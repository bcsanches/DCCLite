// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CargoProducer.h"

#include <fmt/chrono.h>

#include <dcclite/JsonUtils.h>
#include <dcclite/FmtUtils.h>

#include "../Cargo.h"
#include "../FastClockUtils.h"
#include "../Industry.h"
#include "../TycoonService.h"

namespace dcclite::broker::tycoon::detail
{	
	///////////////////////////////////////////////////////////////////////////
	//
	//
	// CargoProducer
	//
	//
	///////////////////////////////////////////////////////////////////////////
	CargoProducer::CargoProducer(TycoonService &tycoon, Industry &industry, const rapidjson::Value &params) :
		m_clProductionThinker{ tycoon.GetFastClock().MakeThinker("CargoProducer::ProduceThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker)) },
		m_rclIndustry{ industry },
		m_fpDailyRate{ json::GetFloat(params, "dailyProduction", "[Tycoon::CargoHolder]") },
		m_uMaxQuantity{ static_cast<uint8_t>(json::GetRangedInt(params, "maximumStorage", 0, 255, "[CargoProducer]")) }
	{
		if (m_fpDailyRate <= 0.0f)
		{
			throw std::invalid_argument(fmt::format("[CargoProducer::CargoProducer] [{}]: dailyRate must be greater than zero", m_rclIndustry.GetName()));
		}

		if (m_uMaxQuantity == 0)
		{
			throw std::invalid_argument(fmt::format("[CargoProducer::CargoProducer] [{}]: maxQuantity must be greater than zero", m_rclIndustry.GetName()));
		}

		m_clProductionManager.Load(tycoon, params, m_rclIndustry.GetName());

		this->ScheduleProduction(tycoon.GetFastClock().Now());
	}

	void CargoProducer::ScheduleProduction(const FastClock::time_point now)
	{
		m_fProducing = true;

		constexpr auto secondsPerDay = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{ 24 });
		auto interval = std::chrono::seconds{ static_cast<long>(secondsPerDay.count() / m_fpDailyRate) };

		m_clProductionThinker.Schedule(now + interval);

		//auto realTimeInverval = fastClock.ConvertToRealTime(interval);
		//dcclite::Log::Trace("[Tycoon::CargoProducer::ScheduleProduction] Next production in ~{} (~{})", realTimeInverval, std::chrono::duration_cast<std::chrono::minutes>(realTimeInverval));
	}


	std::chrono::hours CargoProducer::StartSpotLoad(Spot &spot, RName cargoName)
	{
		const auto cargoInfoIndex = m_clProductionManager.FindCargoInfoIndexByCargoName(cargoName, m_rclIndustry.GetName());
		auto &cargoInfo = m_clProductionManager.GetCargoInfo(cargoInfoIndex);

		//make sure spot will not throw after we call StartCargoTransfer on cargo holder, 
		// otherwise we will have an inconsistent state where cargo is reserved but spot is not loading
		if (!spot.CanLoad())
			throw std::runtime_error(fmt::format("[CargoProducer::StartSpotLoad] [{}]: Spot {} cannot be loaded because it is not reserved", m_rclIndustry.GetName(), spot.GetName()));

		//Start cargo transfer first, because it can throw if the transfer cannot be started,
		auto transferTime = cargoInfo.StartCargoTransfer();

		//As we checked CanLoad() above, this should not throw...
		spot.Load(cargoInfoIndex);

		dcclite::Log::Trace("[CargoProducer::StartSpotLoad] [{}]: Started loading car at spot {} with {}, transfer will take {}",
			m_rclIndustry.GetName(),
			cargoInfo.GetCargo().GetName(),
			spot.GetName(),
			transferTime
		);

		return transferTime;
	}

	void CargoProducer::FinishSpotTransfer(Spot &spot, const FastClock::time_point now)
	{
		auto cargoInfoIndex = spot.GetCargoIndex().value();

		auto &cargoInfo = m_clProductionManager.GetCargoInfo(cargoInfoIndex);

		//make sure spot will not throw, otherwise we will have an inconsistent state where transfer is completed but spot is still loading/unloading
		if (!spot.CanCompleteCargoTransfer())
			throw std::runtime_error(fmt::format("[CargoProducer::OnCompleteSpotTransfer] [{}]: Spot {} cannot complete transfer because it is not loading or unloading", m_rclIndustry.GetName(), spot.GetName()));

		//do cargo info op first, because it can throw if the transfer cannot be completed, 
		// and we dont want to have a spot that is still loading/unloading but the cargo transfer is completed
		spot.OnCompleteCargoTransfer(cargoInfo.CompleteCargoTransfer());

		dcclite::Log::Trace("[CargoProducer::OnCompleteSpotTransfer] [{}]: Spot {} finished transfer of {}", m_rclIndustry.GetName(), spot.GetName(), cargoInfo.GetCargo().GetName());

		//If not producing and if we have room, resume production...
		//Checking TotalCargoStored against m_uMaxQuantity seems redundant, but, if the user changes the MaximumStorage
		//and we load a state with a maximum storage lower than we have stored, this may happen, so we avoid restarting production 
		//untill all excess cargo is consumed...
		if ((!m_fProducing) && (this->CalculateTotalCargoStored() < m_uMaxQuantity))
		{
			//if production was halted because we reached max capacity, try to resume it now that we have free storage
			this->ScheduleProduction(now);
		}
	}

	void CargoProducer::ProduceThinker(FastClockDef::TimePoint_t tp)
	{
		auto cargoIndex = m_clProductionManager.RandomSelectCargoToProduce(m_rclIndustry.GetName());
		auto &cargoInfo = m_clProductionManager.GetCargoInfo(cargoIndex);

		cargoInfo.IncreaseQuantity();

		auto totalStockQuantity = this->CalculateTotalCargoStored();

		dcclite::Log::Trace("[Tycoon::CargoProducer::ScheduleProduction] [{}]: Produced {}, total storage is: {}", m_rclIndustry.GetName(), cargoInfo.GetCargo().GetName(), totalStockQuantity);

		if (totalStockQuantity >= m_uMaxQuantity)
		{
			dcclite::Log::Trace("[Tycoon::CargoProducer::ScheduleProduction] [{}]: Stock is full, halted production", m_rclIndustry.GetName());

			//reached max capacity, stop production
			m_fProducing = false;
		}
		else
		{
			this->ScheduleProduction(tp);
		}

		m_rclIndustry.OnCargoProduced(AccessToken<CargoProducer>{}, cargoIndex);
	}

	[[nodiscard]] CargoQuantity CargoProducer::GetCargoQuantity(RName cargoName) const
	{
		return m_clProductionManager.GetCargoQuantity(cargoName, m_rclIndustry.GetName());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Cargo Producer Serialization Procs
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CargoProducer::Serialize(dcclite::JsonOutputStream_t &stream, const FastClock &fastClock) const
	{
		stream.AddFloatValue("dailyRate", m_fpDailyRate);
		stream.AddIntValue("maximumQuantity", m_uMaxQuantity);

		this->SerializeDeltaDataOnly(stream, fastClock);

		m_clProductionManager.Serialize(stream);
	}

	void CargoProducer::SerializeDeltaDataOnly(dcclite::JsonOutputStream_t &stream, const FastClock &fastClock) const
	{
		stream.AddBool("producing", m_fProducing);

		if (m_fProducing)
		{
			auto nextProductionTime = m_clProductionThinker.GetTimePoint();

			auto localTime = FastClockUtils::GetLocalTime(nextProductionTime, fastClock);

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

	void CargoProducer::SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, CargoIndex cargoInfoIndex) const
	{
		auto cargoInfoObject = stream.AddObject("cargoInfo");
		m_clProductionManager.SerializeCargoInfoDelta(cargoInfoObject, cargoInfoIndex);
	}	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Cargo Producer Save / Load
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CargoProducer::SaveState(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddBool("producing", m_fProducing);

		stream.AddInt64Value("productionTimePoint", FastClockDef::ConvertToIntMs(m_clProductionThinker.GetTimePoint()));

		{
			auto producesData = stream.AddObject("production");
			
			m_clProductionManager.SaveState(producesData);
		}
	}

	bool CargoProducer::LoadState(const rapidjson::Value &params, const FastClock::time_point now)
	{
		m_fProducing = json::GetBool(params, "producing", "[Industry::LoadState]");

		if (m_fProducing)
		{
			auto timePoint = json::GetInt64(params, "productionTimePoint", "[Industry::LoadState]");
			m_clProductionThinker.Schedule(FastClockDef::ConvertFromIntMs(timePoint));
		}
		else
		{
			//Constructor may have started production, halt it...
			m_clProductionThinker.Cancel();
		}

		auto productionData = json::TryGetObject(params, "production");
		if (productionData)
		{
			if(!m_clProductionManager.LoadState(*productionData))
			{
				dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: Failed to load production state", m_rclIndustry.GetName());
				return false;
			}			
		}
		else
		{
			//This should never happens...
			dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: production state not found, state is probably corrupted", m_rclIndustry.GetName());

			return false;
		}

		auto total = this->CalculateTotalCargoStored();
		if (total > m_uMaxQuantity)
		{
			dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: Total cargo stored {} exceeds max quantity {}, state is probably corrupted", m_rclIndustry.GetName(), total, m_uMaxQuantity);

			//if production was under way, cancel it...
			if (m_fProducing)
			{
				m_clProductionThinker.Cancel();
				m_fProducing = false;
			}
		}
		else if ((total < m_uMaxQuantity) && (!m_fProducing))
		{
			//if we are not producing but we have free storage, try to resume production
			this->ScheduleProduction(now);
		}

		return true;
	}

	void CargoProducer::ResetState(const FastClock::time_point now)
	{
		m_clProductionManager.ResetState();

		if (!m_fProducing)
			this->ScheduleProduction(now);
	}
}