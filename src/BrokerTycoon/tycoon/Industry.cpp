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

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>

#include <fmt/chrono.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/RName.h>
#include <dcclite/JsonUtils.h>

#include "Cargo.h"
#include "FastClockUtils.h"
#include "TycoonService.h"

static thread_local std::mt19937 g_clRandomGenerator{ std::random_device{}() };

namespace dcclite::broker::tycoon
{	
	///////////////////////////////////////////////////////////////////////////
	//
	//
	// Industry
	//
	//
	///////////////////////////////////////////////////////////////////////////

	const char *Industry::TYPE_NAME = "dcclite::broker::tycoon::Industry";	

	Industry::Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params):
		Object{name},
		m_clProductionThinker{ tycoon.GetFastClock().MakeThinker("Industry::ProduceThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker))},
		m_rclTycoon{ tycoon },
		m_fpDailyRate{ json::GetFloat(params, "dailyProduction", "[Tycoon::CargoHolder]") },
		m_uMaxQuantity{ static_cast<uint8_t>(json::GetRangedInt(params, "maximumStorage", 0, 255, "[CargoHolder]")) }
	{
		if (m_fpDailyRate <= 0.0f)
		{
			throw std::invalid_argument("[CargoHolder::CargoHolder] dailyRate must be greater than zero");
		}

		if (m_uMaxQuantity == 0)
		{
			throw std::invalid_argument("[CargoHolder::CargoHolder] maxQuantity must be greater than zero");
		}

		this->LoadProductionData(params);
		this->LoadSpots(params);

		this->AdjustProductionChances();

		this->ScheduleProduction();
	}

	void Industry::LoadProduce(const rapidjson::Value &params)
	{
		m_vecProduces.emplace_back(m_rclTycoon, params);
	}

	void Industry::LoadProductionData(const rapidjson::Value &params)
	{
		auto singleProduce = params.FindMember("produce");
		if (singleProduce != params.MemberEnd())
		{
			if (!singleProduce->value.IsObject())
			{
				throw std::invalid_argument("[Industry::Industry] produce must be an object");
			}

			this->LoadProduce(singleProduce->value);
		}

		auto producesValue = params.FindMember("produces");
		if (producesValue == params.MemberEnd())
		{
			if (!m_vecProduces.empty())
			{
				return;
			}

			throw std::invalid_argument("[Industry::Industry] either produce or produces must be specified");
		}

		if (producesValue->value.IsObject())
		{
			//redudant... but...
			this->LoadProduce(producesValue->value);

			return;
		}

		if (!producesValue->value.IsArray())
		{
			throw std::invalid_argument("[Industry::Industry] produces must be either an object or an array");
		}

		auto producesData = producesValue->value.GetArray();
		m_vecProduces.reserve(producesData.Size());
		for (auto &it : producesData)
		{
			if (!it.IsObject())
			{
				throw std::invalid_argument("[Industry::Industry] each produce in produces array must be an object");
			}

			this->LoadProduce(it);
		}

		if (m_vecProduces.empty())
		{
			throw std::invalid_argument("[Industry::Industry] at least one produce must be specified");
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
				throw std::invalid_argument(fmt::format("[Industry::Industry] multiple produces with the same cargo {} are not allowed", m_vecProduces[i].GetCargo().GetName()));
			}
		}
	}

	void Industry::LoadSpots(const rapidjson::Value &params)
	{
		if (auto spot = json::TryGetString(params, "spot"))
		{
			this->AddSpot(RName{ *spot });
		}

		auto spotsValue = params.FindMember("spots");
		if (spotsValue == params.MemberEnd())
		{
			if (m_vecSpots.empty())
			{
				throw std::invalid_argument("[Industry::Industry] either spot or spots must be specified");
			}

			return;
		}

		if (!m_vecSpots.empty())
		{
			throw std::invalid_argument("[Industry::Industry] spot and spots cannot be specified at the same time");
		}

		const auto spotsNum = spotsValue->value.GetArray().Size();
		if (spotsNum == 0)
		{
			throw std::invalid_argument("[Industry::Industry] at least one spot must be specified");
		}

		m_vecSpots.reserve(spotsNum);
		m_vecSpotThinkers.reserve(spotsNum);

		for (auto &it : spotsValue->value.GetArray())
		{
			if (!it.IsString())
			{
				throw std::invalid_argument("[Industry::Industry] each spot must be a string");
			}

			this->AddSpot(RName{ it.GetString() });
		}
	}

	void Industry::AdjustProductionChances()
	{		
		m_uProduceTotalChance = 0;
		for (auto i = 0; i < m_vecProduces.size(); ++i)
		{			
			m_vecProduces[i].SetSequence(m_vecProduces[i].GetChance() + m_uProduceTotalChance);
			m_uProduceTotalChance += m_vecProduces[i].GetChance();
		}
	}

	void Industry::AddSpot(RName name)
	{
		m_vecSpots.emplace_back(name);

		m_vecSpotThinkers.push_back(m_rclTycoon.GetFastClock().MakeUniqueThinker(
			this->GetNameData(),
			[this, spotIndex = m_vecSpots.size() - 1](FastClockDef::TimePoint_t tp)
			{
				this->OnCompleteSpotTransfer(tp, spotIndex);
			})
		);
	}	

	std::optional<size_t> Industry::TryFindSpotIndex(RName spotName) const
	{
		auto it = std::ranges::find_if(m_vecSpots, [spotName](const detail::Spot &s) { return s.GetName() == spotName; });
		if (it == m_vecSpots.end())
		{
			return std::nullopt;
		}

		return std::distance(m_vecSpots.begin(), it);
	}

	size_t Industry::FindSpotIndex(RName spotName) const
	{
		auto index = this->TryFindSpotIndex(spotName);
		if(!index)
			throw std::runtime_error(fmt::format("[Industry::FindSpotIndex] Spot {} not found in industry {}", spotName, this->GetName()));

		return *index;
	}

	detail::Spot *Industry::TryFindSpot(RName spotName)
	{
		auto index = this->TryFindSpotIndex(spotName);
		
		return index ? &m_vecSpots[*index] : nullptr;		
	}

	detail::Spot &Industry::FindSpot(RName spotName)
	{
		auto spot = this->TryFindSpot(spotName);
		if(spot == nullptr)
			throw std::runtime_error(fmt::format("[Industry::FindSpot] Spot {} not found in industry {}", spotName, this->GetName()));

		return *spot;
	}

	void Industry::SendSpotStateChangedEvent(const detail::Spot &spot) const
	{
		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this, [this, &spot](JsonOutputStream_t &stream)
			{
				//just send down the spot that changed...
				this->SerializeIdentification(stream);

				auto spotsData = stream.AddArray("spots");
				auto spotObject = spotsData.AddObject();
				spot.Serialize(spotObject);
			}
		);
	}	

	void Industry::ReserveSpot(RName spotName, const char *info)
	{
		auto &spot = this->FindSpot(spotName);
		
		spot.Reserve(info);

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::CancelSpotReservation(RName spotName)
	{
		auto &spot = this->FindSpot(spotName);		

		spot.CancelReservation();

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::StartSpotLoad(RName spotName, RName cargoName)
	{
		const auto cargoInfoIndex = this->FindCargoInfoIndexByCargoName(cargoName);
		auto &cargoInfo = m_vecProduces[cargoInfoIndex];

		const auto spotIndex = this->FindSpotIndex(spotName);	
		auto &spot = m_vecSpots[spotIndex];

		//make sure spot will not throw after we call StartCargoTransfer on cargo holder, 
		// otherwise we will have an inconsistent state where cargo is reserved but spot is not loading
		if(!spot.CanLoad())
			throw std::runtime_error(fmt::format("[Industry::StartSpotLoad] Spot {} cannot be loaded because it is not reserved", spotName));

		auto transferTime = cargoInfo.StartCargoTransfer();
		spot.Load((int)cargoInfoIndex);

		dcclite::Log::Trace("[Industry::StartSpotLoad] {}: Started loading car at spot {} with {}, transfer will take {}", 
			this->GetName(), 
			cargoInfo.GetCargo().GetName(),
			spotName, 
			transferTime
		);

		m_vecSpotThinkers[spotIndex]->Schedule(m_rclTycoon.GetFastClock().Now() + transferTime);

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}

	void Industry::RemoveCarFromSpot(RName spotName)
	{
		auto &spot = this->FindSpot(spotName);

		spot.RemoveCar();

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::OnCompleteSpotTransfer(FastClockDef::TimePoint_t tp, size_t spotIndex)
	{
		assert(spotIndex < m_vecSpots.size());		

		auto &spot = m_vecSpots[spotIndex];
		auto cargoInfoIndex = spot.GetCargoIndex();

		if((cargoInfoIndex < 0) || (cargoInfoIndex >= m_vecProduces.size()))
		{
			dcclite::Log::Error("[Industry::OnCompleteSpotTransfer] Spot {} has invalid cargo index {}, cannot complete transfer", spot.GetName(), cargoInfoIndex);
			return;
		}

		auto &cargoInfo = m_vecProduces[cargoInfoIndex];

		//make sure spot will not throw, otherwise we will have an inconsistent state where transfer is completed but spot is still loading/unloading
		if(!spot.CanCompleteCargoTransfer())
			throw std::runtime_error(fmt::format("[Industry::OnCompleteSpotTransfer] Spot {} cannot complete transfer because it is not loading or unloading", spot.GetName()));

		//do cargo info op first, because it can throw if the transfer cannot be completed, 
		// and we dont want to have a spot that is still loading/unloading but the cargo transfer is completed
		cargoInfo.CompleteCargoTransfer();
		spot.OnCompleteCargoTransfer();		

		dcclite::Log::Trace("[Industry::OnCompleteSpotTransfer] {}: Spot {} finished transfer of {}", this->GetName(), spot.GetName(), cargoInfo.GetCargo().GetName());

		if (!m_fProducing)
		{
			//if production was halted because we reached max capacity, try to resume it now that we have free storage
			this->ScheduleProduction();
		}

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}

	const Cargo *Industry::TryGetCargoByCargoInfoIndex(size_t index) const
	{
		if(index >= m_vecProduces.size())
			return nullptr;

		return &m_vecProduces[index].GetCargo();
	}

	int Industry::TryGetCargoInfoIndexByCargoName(std::string_view name) const
	{
		RName cargoName = RName::TryGetName(name);
		if (!cargoName)
			return -1;

		auto it = std::ranges::find_if(
			m_vecProduces,
			[cargoName](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == cargoName; }
		);

		if(it == m_vecProduces.end())
			return -1;

		return static_cast<int>(std::distance(m_vecProduces.begin(), it));
	}

	size_t Industry::FindCargoInfoIndexByCargoName(RName cargoName) const
	{		
		auto it = std::ranges::find_if(
			m_vecProduces, 
			[cargoName](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == cargoName; }
		);

		if(it == m_vecProduces.end())
			throw std::runtime_error(fmt::format("[Industry::FindCargoInfoIndexByCargoName] Cargo {} not found in industry {}", cargoName, this->GetName()));

		return std::distance(m_vecProduces.begin(), it);
	}

	unsigned Industry::CalculateTotalCargoStored() const noexcept
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

	size_t Industry::RandomSelectCargoToProduce() const noexcept
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

		dcclite::Log::Error("[Tycoon::Industry::RandomSelectCargoToProduce] Could not find a cargo for industry {}", this->GetName());

		std::uniform_int_distribution<> safeDist(0, (int)m_vecProduces.size() - 1);

		return (size_t)safeDist(g_clRandomGenerator);
	}

	void Industry::ScheduleProduction()
	{
		m_fProducing = true;

		auto &fastClock = m_rclTycoon.GetFastClock();

		auto now = fastClock.Now();

		constexpr auto secondsPerDay = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{ 24 });
		auto interval = std::chrono::seconds{ static_cast<long>(secondsPerDay.count() / m_fpDailyRate) };

		m_clProductionThinker.Schedule(now + interval);

		auto realTimeInverval = fastClock.ConvertToRealTime(interval);
		dcclite::Log::Trace("[Tycoon::Industry::ScheduleProduction] {}: Next production in ~{} (~{})", this->GetName(), realTimeInverval, std::chrono::duration_cast<std::chrono::minutes>(realTimeInverval));
	}

	void Industry::ProduceThinker(FastClockDef::TimePoint_t tp)
	{
		auto cargoIndex = (int)this->RandomSelectCargoToProduce();
		auto &cargoInfo = m_vecProduces[cargoIndex];

		cargoInfo.IncreaseQuantity();

		auto totalStockQuantity = this->CalculateTotalCargoStored();

		dcclite::Log::Trace("[Tycoon::Industry::ScheduleProduction] {}: Produced {}, total storage is: {}", this->GetName(), cargoInfo.GetCargo().GetName(), totalStockQuantity);

		if (totalStockQuantity >= m_uMaxQuantity)
		{
			dcclite::Log::Trace("[Tycoon::Industry::ScheduleProduction] {}: Stock is full, halted production", this->GetName());

			//reached max capacity, stop production
			m_fProducing = false;
		}
		else
		{
			this->ScheduleProduction();
		}

		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this, [this, cargoIndex](JsonOutputStream_t &stream) { this->SerializeDelta(stream, cargoIndex); });
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Serialization
	// 
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Industry::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Object::Serialize(stream);

		stream.AddFloatValue("dailyRate", m_fpDailyRate);
		stream.AddIntValue("maximumQuantity", m_uMaxQuantity);

		this->SerializeDeltaDataOnly(stream);

		{
			auto cargoInfoData = stream.AddArray("produces");
			for (auto it : m_vecProduces)
			{
				auto obj = cargoInfoData.AddObject();

				it.Serialize(obj);
			}
		}

		auto spotsData = stream.AddArray("spots");
		for (auto &spot : m_vecSpots)
		{
			auto spotObject = spotsData.AddObject();
			spot.Serialize(spotObject);
		}
	}

	void Industry::SerializeDeltaDataOnly(dcclite::JsonOutputStream_t &stream) const
	{
		stream.AddBool("producing", m_fProducing);

		if (m_fProducing)
		{
			auto nextProductionTime = m_clProductionThinker.GetTimePoint();

			auto localTime = FastClockUtils::GetLocalTime(nextProductionTime, m_rclTycoon.GetFastClock());

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

	void Industry::SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, const int cargoInfoIndex) const
	{
		auto cargoInfoObject = stream.AddObject("cargoInfo");
		m_vecProduces[cargoInfoIndex].SerializeDelta(cargoInfoObject);
		cargoInfoObject.AddIntValue("index", cargoInfoIndex);
	}

	void Industry::SerializeDelta(dcclite::JsonOutputStream_t &stream, int cargoInfoHintIndex) const
	{
		this->SerializeIdentification(stream);
		this->SerializeDeltaDataOnly(stream);

		if (cargoInfoHintIndex < 0)
		{
			auto cargoInfoData = stream.AddArray("produces");
			for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
			{
				auto obj = cargoInfoData.AddObject();

				m_vecProduces[i].SerializeDelta(obj);
			}
		}
		else
		{
			this->SerializeCargoInfo(stream, cargoInfoHintIndex);
		}
	}

	void Industry::SendDeltaWithSpotStateChangedEvent(const detail::Spot &spot) const
	{
		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this, [this, &spot](JsonOutputStream_t &stream)
			{
				//just send down the delta, including cargo holder, because its state changed...
				this->SerializeDelta(stream);

				{
					auto spotsData = stream.AddArray("spots");
					auto spotObject = spotsData.AddObject();
					spot.Serialize(spotObject);
				}

				//if the spot has cargo info, send it also...
				auto cargoInfoIndex = spot.GetCargoIndex();
				if (cargoInfoIndex >= 0)
				{
					this->SerializeCargoInfo(stream, cargoInfoIndex);
				}
			}
		);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//
	// Save / Load states
	// 
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Industry::SaveState(dcclite::JsonOutputStream_t &stream) const
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

		{
			auto spotsData = stream.AddObject("spots");

			for (size_t i = 0, sz = m_vecSpots.size(); i < sz; ++i)
			{
				auto spotData = spotsData.AddObject(m_vecSpots[i].GetNameData());

				spotData.AddInt64Value("transferTimePoint", FastClockDef::ConvertToIntMs(m_vecSpotThinkers[i]->GetTimePoint()));

				auto spotObjectData = spotData.AddObject("spot");
				m_vecSpots[i].SaveState(spotObjectData, *this);
			}
		}		
	}

	void Industry::LoadState(const rapidjson::Value &params)
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

		{
			auto productionData = json::TryGetObject(params, "production");
			if (productionData)
			{
				for (auto &it : productionData->GetObject())
				{
					auto cargoNameStr = std::string_view{ it.name.GetString(), it.name.GetStringLength() };
					auto cargoName = RName::TryGetName(cargoNameStr);
					if (!cargoName)
					{
						dcclite::Log::Warn("[Industry::LoadState] [{}] Cargo {} name is not even registered, skipping", this->GetName(), cargoNameStr);

						//This is not so bad, we can live with that, maybe user removed a product from industry after the state was saved...
						continue;
					}

					auto cargoInfoIndex = this->FindCargoInfoIndexByCargoName(cargoName);
					if (cargoInfoIndex < 0)
					{
						dcclite::Log::Warn("[Industry::LoadState] [{}] Cargo {} in production state not found, skipping", this->GetName(), cargoName);

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
						dcclite::Log::Warn("[Industry::LoadState] [{}] Cargo {} was not found in production state", this->GetName(), cargoName);

						goto CORRUPTED_STATE;
					}
				}
			}
			else if (!m_vecProduces.empty())
			{
				dcclite::Log::Warn("[Industry::LoadState] [{}] production state not found, but industry has produces, state is probably corrupted", this->GetName());

				goto CORRUPTED_STATE;
			}
		}

		{
			auto &spotsData = json::GetObject(params, "spots", "[Industry::LoadState]");
			for (auto &it : spotsData.GetObject())
			{
				auto spotName = it.name.GetString();

				RName rname = RName::TryGetName(spotName);
				if (!rname)
				{
					dcclite::Log::Warn("[Industry::LoadState] [{}] Spot {} name is not even registered, skipping", this->GetName(), spotName);

					goto CORRUPTED_STATE;
				}

				auto spotIndex = this->TryFindSpotIndex(rname);
				if (!spotIndex)
				{
					dcclite::Log::Warn("[Industry::LoadState] [{}] Spot {} not found in state file, skipping", this->GetName(), spotName);

					goto CORRUPTED_STATE;
				}

				auto &spotThinker = m_vecSpotThinkers[*spotIndex];
				auto &spot = m_vecSpots[*spotIndex];

				auto spotData = it.value.GetObject();

				auto &spotObjectData = json::GetObject(spotData, "spot", "[Industry::LoadState]");

				if (!spot.LoadState(spotObjectData, *this))
				{
					dcclite::Log::Warn("[Industry::LoadState] [{}]  thisFailed to load state for spot {}, state is probably corrupted", this->GetName(), spotName);

					goto CORRUPTED_STATE;
				}

				if (spot.IsTransfering())
				{
					auto transferTimePoint = json::GetInt64(spotData, "transferTimePoint", "[Industry::LoadState]");
					spotThinker->Schedule(FastClockDef::ConvertFromIntMs(transferTimePoint));
				}
			}
		}

		{
			auto total = this->CalculateTotalCargoStored();
			if (total > m_uMaxQuantity)
			{
				dcclite::Log::Warn("[Industry::LoadState] [{}] Total cargo stored {} exceeds max quantity {}, state is probably corrupted", this->GetName(), total, m_uMaxQuantity);

				goto CORRUPTED_STATE;
			}
			else if ((total < m_uMaxQuantity) && (!m_fProducing))
			{
				//if we are not producing but we have free storage, try to resume production
				this->ScheduleProduction();
			}
		}

		return;

CORRUPTED_STATE:

		//try to fix state
		dcclite::Log::Warn("[Industry::LoadState] [{}] State corrupted, resetting it to initial state, sorry...", this->GetName());

		std::ranges::for_each(m_vecProduces, [](detail::CargoInfo &ci) { ci.Reset(); });
			
		for (size_t i = 0, sz = m_vecSpots.size(); i < sz; ++i)
		{
			m_vecSpots[i].Reset();
			m_vecSpotThinkers[i]->Cancel();
		}

		if(!m_fProducing)
			this->ScheduleProduction();		
	}
}

