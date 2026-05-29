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

#include <algorithm>
#include <numeric>
#include <random>

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

static thread_local std::mt19937 g_clRandomGenerator{ std::random_device{}() };

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
		stream.AddStringValue("cargoInformation", m_strCargoInformation);
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

	void Spot::Reset()
	{
		m_kState = SpotStates::FREE;
		m_strInformation.clear();
		m_iCargoIndex = -1;
	}

	void Spot::SaveState(dcclite::JsonOutputStream_t &stream, const Industry &industry) const
	{
		stream.AddStringValue("state", magic_enum::enum_name(m_kState));
		stream.AddStringValue("info", m_strInformation);
		stream.AddStringValue("carfoInformation", m_strCargoInformation);

		if (m_iCargoIndex >= 0)
			stream.AddStringValue("cargo", industry.TryGetCargoByCargoInfoIndex(m_iCargoIndex)->GetNameData());
	}

	std::optional<SpotStates> Spot::LoadStateEnum(const rapidjson::Value &params, const char *field)
	{
		auto state = magic_enum::enum_cast<SpotStates>(dcclite::json::GetValue(params, field, "[Spot::LoadState]").GetString());
		if (!state)
			return {};

		return state;
	}

	bool Spot::LoadState(const rapidjson::Value &params, const Industry &industry)
	{
		auto state = Spot::LoadStateEnum(params, "state");		
		if(!state)
		{
			throw std::invalid_argument(fmt::format("[Spot::LoadState] Invalid state value: {}", dcclite::json::GetValue(params, "state", "[Spot::LoadState]").GetString()));
		}

		m_kState = *state;

		if (auto info = json::TryGetString(params, "info"))
			m_strInformation = *info;

		if (auto cargoInfo = json::TryGetString(params, "cargoInformation"))
			m_strCargoInformation = *cargoInfo;

		if (auto cargoName = json::TryGetString(params, "cargo"))
		{
			auto index = industry.TryGetCargoInfoIndexByCargoName(cargoName.value());
			if (index < 0)
			{
				dcclite::Log::Error("[Spot::LoadState] Invalid cargo name: {}, resetting spot", cargoName.value());

				m_iCargoIndex = -1;
				m_strInformation.clear();
				m_kState = SpotStates::FREE;

				return false;
			}

			m_iCargoIndex = index;
		}

		return true;
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

	const std::string &CargoInfo::CompleteCargoTransfer()
	{
		if (m_uReservedQuantity == 0)
		{
			throw std::runtime_error("[Tycoon::CargoInfo::CompleteCargoTransfer] No cargo reserved!!!");
		}

		--m_uReservedQuantity;

		std::uniform_int_distribution<> dist(0, (int)m_vecDestinations.size() - 1);

		unsigned destination = dist(g_clRandomGenerator);

		return m_vecDestinations[destination];
	}

	void CargoInfo::Reset()
	{
		m_uReservedQuantity = 0;
		m_uCurrentQuantity = 0;
	}

	void CargoInfo::SaveState(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddIntValue("currentQuantity", m_uCurrentQuantity);
		stream.AddIntValue("reservedQuantity", m_uReservedQuantity);
	}

	void CargoInfo::LoadState(const rapidjson::Value &params)
	{
		m_uCurrentQuantity = json::GetInt(params, "currentQuantity", "[CargoInfo::LoadState]");
		m_uReservedQuantity = json::GetInt(params, "reservedQuantity", "[CargoInfo::LoadState]");
	}

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

		this->LoadProductionData(tycoon, params);
		this->AdjustProductionChances();

		this->ScheduleProduction(tycoon.GetFastClock().Now());
	}


	void CargoProducer::LoadProduce(TycoonService &tycoon, const rapidjson::Value &params)
	{
		m_vecProduces.emplace_back(tycoon, params);
	}

	void CargoProducer::LoadProductionData(TycoonService &tycoon, const rapidjson::Value &params)
	{
		auto singleProduce = params.FindMember("produce");
		if (singleProduce != params.MemberEnd())
		{
			if (!singleProduce->value.IsObject())
			{
				throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: produce must be an object", m_rclIndustry.GetName()));
			}

			this->LoadProduce(tycoon, singleProduce->value);
		}

		auto producesValue = params.FindMember("produces");
		if (producesValue == params.MemberEnd())
		{
			if (!m_vecProduces.empty())
			{
				return;
			}

			throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: either produce or produces must be specified", m_rclIndustry.GetName()));
		}

		if (producesValue->value.IsObject())
		{
			//redudant... but...
			this->LoadProduce(tycoon, producesValue->value);

			return;
		}

		if (!producesValue->value.IsArray())
		{
			throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: produces must be either an object or an array", m_rclIndustry.GetName()));
		}

		auto producesData = producesValue->value.GetArray();
		m_vecProduces.reserve(producesData.Size());
		for (auto &it : producesData)
		{
			if (!it.IsObject())
			{
				throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: each produce in produces array must be an object", m_rclIndustry.GetName()));
			}

			this->LoadProduce(tycoon, it);
		}

		if (m_vecProduces.empty())
		{
			throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: at least one produce must be specified", m_rclIndustry.GetName()));
		}

		//
		//make sure we have a single type
		for (size_t i = 0, sz = m_vecProduces.size() - 1; i < sz; ++i)
		{
			auto it = std::find_if(
				m_vecProduces.begin() + i + 1,
				m_vecProduces.end(),
				[this, i](const detail::CargoInfo &ci)
				{
					return &ci.GetCargo() == &m_vecProduces[i].GetCargo();
				}
			);

			if (it != m_vecProduces.end())
			{
				throw std::invalid_argument(fmt::format("[CargoProducer::LoadProductionData] [{}]: multiple produces with the same cargo {} are not allowed", m_rclIndustry.GetName(), m_vecProduces[i].GetCargo().GetName()));
			}
		}
	}

	void CargoProducer::AdjustProductionChances()
	{
		m_uProduceTotalChance = 0;
		for (auto i = 0; i < m_vecProduces.size(); ++i)
		{
			m_vecProduces[i].SetSequence(m_vecProduces[i].GetChance() + m_uProduceTotalChance);
			m_uProduceTotalChance += m_vecProduces[i].GetChance();
		}
	}

	unsigned CargoProducer::CalculateTotalCargoStored() const noexcept
	{
		return std::accumulate(
			m_vecProduces.begin(),
			m_vecProduces.end(),
			0u,
			[](unsigned acc, const detail::CargoInfo &info)
			{
				return acc + info.GetTotal();
			}
		);
	}

	const Cargo *CargoProducer::TryGetCargoByCargoInfoIndex(size_t index) const noexcept
	{
		if (index >= m_vecProduces.size())
			return nullptr;

		return &m_vecProduces[index].GetCargo();
	}

	int CargoProducer::TryGetCargoInfoIndexByCargoName(RName rname) const noexcept
	{
		auto it = std::ranges::find_if(
			m_vecProduces,
			[rname](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == rname; }
		);

		if (it == m_vecProduces.end())
			return -1;

		return static_cast<int>(std::distance(m_vecProduces.begin(), it));
	}

	int CargoProducer::TryGetCargoInfoIndexByCargoName(std::string_view name) const noexcept
	{
		RName cargoName = RName::TryGetName(name);
		if (!cargoName)
			return -1;

		return this->TryGetCargoInfoIndexByCargoName(cargoName);		
	}

	size_t CargoProducer::FindCargoInfoIndexByCargoName(RName cargoName) const
	{
		auto it = std::ranges::find_if(
			m_vecProduces,
			[cargoName](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == cargoName; }
		);

		if (it == m_vecProduces.end())
			throw std::runtime_error(fmt::format("[CargoProducer::FindCargoInfoIndexByCargoName] [{}]: Cargo {} not found", m_rclIndustry.GetName(), cargoName));

		return std::distance(m_vecProduces.begin(), it);
	}

	size_t CargoProducer::RandomSelectCargoToProduce() const noexcept
	{
		if (m_vecProduces.size() == 1)
			return 0;

		std::uniform_int_distribution<> dist(0, m_uProduceTotalChance - 1);

		unsigned chance = dist(g_clRandomGenerator);

		for (unsigned i = 0, previousSequence = 0; i < m_vecProduces.size(); ++i)
		{
			auto &cargoInfo = m_vecProduces[i];
			auto sequence = cargoInfo.GetSequence();
			if ((chance >= previousSequence) && (chance < sequence))
				return i;

			previousSequence = sequence;
		}

		dcclite::Log::Error("[Tycoon::CargoProducer::RandomSelectCargoToProduce] [{}]: Could not find cargo", m_rclIndustry.GetName());

		std::uniform_int_distribution<> safeDist(0, (int)m_vecProduces.size() - 1);

		return (size_t)safeDist(g_clRandomGenerator);
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
		const auto cargoInfoIndex = this->FindCargoInfoIndexByCargoName(cargoName);
		auto &cargoInfo = this->GetCargoInfo(cargoInfoIndex);		

		//make sure spot will not throw after we call StartCargoTransfer on cargo holder, 
		// otherwise we will have an inconsistent state where cargo is reserved but spot is not loading
		if (!spot.CanLoad())
			throw std::runtime_error(fmt::format("[CargoProducer::StartSpotLoad] [{}]: Spot {} cannot be loaded because it is not reserved", m_rclIndustry.GetName(), spot.GetName()));

		auto transferTime = cargoInfo.StartCargoTransfer();
		spot.Load((int)cargoInfoIndex);

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
		auto cargoInfoIndex = spot.GetCargoIndex();

		if ((cargoInfoIndex < 0) || (cargoInfoIndex >= m_vecProduces.size()))
		{
			throw std::runtime_error(fmt::format("[CargoProducer::FinishSpotTransfer] [{}]: Spot {} has invalid cargo index {}, cannot complete transfer", m_rclIndustry.GetName(), spot.GetName(), cargoInfoIndex));
		}

		auto &cargoInfo = m_vecProduces[cargoInfoIndex];

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
		auto cargoIndex = (int)this->RandomSelectCargoToProduce();
		auto &cargoInfo = m_vecProduces[cargoIndex];

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

	CargoQuantity CargoProducer::GetCargoQuantity(RName cargoName) const
	{
		auto index = this->FindCargoInfoIndexByCargoName(cargoName);

		return { m_vecProduces[index].GetQuantity(), m_vecProduces[index].GetReservedQuantity() };
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

		{
			auto cargoInfoData = stream.AddArray("produces");
			for (auto it : m_vecProduces)
			{
				auto obj = cargoInfoData.AddObject();

				it.Serialize(obj);
			}
		}		
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

	void CargoProducer::SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, const int cargoInfoIndex) const
	{
		auto cargoInfoObject = stream.AddObject("cargoInfo");
		m_vecProduces[cargoInfoIndex].SerializeDelta(cargoInfoObject);
		cargoInfoObject.AddIntValue("index", cargoInfoIndex);
	}

	void CargoProducer::SerializeProductionDelta(dcclite::JsonOutputStream_t &stream) const
	{
		auto cargoInfoData = stream.AddArray("produces");
		for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
		{
			auto obj = cargoInfoData.AddObject();

			m_vecProduces[i].SerializeDelta(obj);
		}
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
			for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
			{
				auto produceData = producesData.AddObject(m_vecProduces[i].GetCargo().GetName().GetData());

				m_vecProduces[i].SaveState(produceData);
			}
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
			for (auto &it : productionData->GetObject())
			{
				auto cargoNameStr = std::string_view{ it.name.GetString(), it.name.GetStringLength() };
				auto cargoName = RName::TryGetName(cargoNameStr);
				if (!cargoName)
				{
					dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: Cargo {} name is not even registered, skipping", m_rclIndustry.GetName(), cargoNameStr);

					//This is not so bad, we can live with that, maybe user removed a product from industry after the state was saved...
					continue;
				}

				auto cargoInfoIndex = this->TryGetCargoInfoIndexByCargoName(cargoName);
				if (cargoInfoIndex < 0)
				{
					dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: Cargo {} in production state not found, skipping", m_rclIndustry.GetName(), cargoName);

					//This is not so bad, we can live with that, maybe user removed a product from industry after the state was saved...
					continue;
				}

				m_vecProduces[cargoInfoIndex].LoadState(it.value);
			}

			//If all production loaded, lets check if user added a new product to this industry
			for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
			{
				auto &cargoInfo = m_vecProduces[i];
				auto cargoName = cargoInfo.GetCargo().GetNameData();
				if (!productionData->HasMember(cargoName.data()))
				{
					dcclite::Log::Warn("[CargoProducer::LoadState] [{}]: Cargo {} was not found in production state", m_rclIndustry.GetName(), cargoName);

					return false;
				}
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
		std::ranges::for_each(m_vecProduces, [](detail::CargoInfo &ci) { ci.Reset(); });

		if (!m_fProducing)
			this->ScheduleProduction(now);
	}
}
