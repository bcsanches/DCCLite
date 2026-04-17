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
		m_clProductionThinker{ tycoon.GetFastClock().MakeThinker("CargoHolder::ProductionThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker))},
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

	void Industry::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Object::Serialize(stream);

		{
			auto cargoInfoData = stream.AddArray("produces");
			for (auto it : m_vecProduces)
			{
				auto obj = cargoInfoData.AddObject();

				it.Serialize(obj);
			}
		}
		
		auto spotsData = stream.AddArray("spots");
		for(auto &spot : m_vecSpots)
		{
			auto spotObject = spotsData.AddObject();
			spot.Serialize(spotObject);			
		}
	}

	void Industry::SerializeDelta(dcclite::JsonOutputStream_t &stream) const
	{
		this->SerializeIdentification(stream);

		{
			auto cargoInfoData = stream.AddArray("produces");
			for (size_t i = 0, sz = m_vecProduces.size(); i < sz; ++i)
			{
				auto obj = cargoInfoData.AddObject();

				m_vecProduces[i].SerializeDelta(obj);
				obj.AddIntValue("index", (int)i);
			}
		}
	}

	std::optional<size_t> Industry::TryFindSpotIndex(const std::string_view spotName) const
	{
		auto it = std::ranges::find_if(m_vecSpots, [spotName](const detail::Spot &s) { return s.GetNameData() == spotName; });
		if (it == m_vecSpots.end())
		{
			return std::nullopt;
		}

		return std::distance(m_vecSpots.begin(), it);
	}

	size_t Industry::FindSpotIndex(const std::string_view spotName) const
	{
		auto index = this->TryFindSpotIndex(spotName);
		if(!index)
			throw std::runtime_error(fmt::format("[Industry::FindSpotIndex] Spot {} not found in industry {}", spotName, this->GetName()));

		return *index;
	}

	detail::Spot *Industry::TryFindSpot(const std::string_view spotName)
	{
		auto index = this->TryFindSpotIndex(spotName);
		
		return index ? &m_vecSpots[*index] : nullptr;		
	}

	detail::Spot &Industry::FindSpot(const std::string_view spotName)
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
				if (auto cargoInfoIndex = spot.GetCargoIndex() >= 0)
				{
					auto cargoInfoObject = stream.AddObject("cargoInfo");
					m_vecProduces[cargoInfoIndex].SerializeDelta(cargoInfoObject);
				}
			}
		);
	}

	void Industry::ReserveSpot(const std::string_view spotName, const char *info)
	{
		auto &spot = this->FindSpot(spotName);
		
		spot.Reserve(info);

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::CancelSpotReservation(const std::string_view spotName)
	{
		auto &spot = this->FindSpot(spotName);		

		spot.CancelReservation();

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::StartSpotLoad(const std::string_view spotName, const std::string_view cargoName)
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

	void Industry::RemoveCarFromSpot(const std::string_view spotName)
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

		auto &cargoInfo = m_vecProduces[cargoInfoIndex];

		//make sure spot will not throw, otherwise we will have an inconsistent state where transfer is completed but spot is still loading/unloading
		if(!spot.CanCompleteCargoTransfer())
			throw std::runtime_error(fmt::format("[Industry::OnCompleteSpotTransfer] Spot {} cannot complete transfer because it is not loading or unloading", spot.GetName()));

		//do cargo info op first, because it can throw if the transfer cannot be completed, 
		// and we dont want to have a spot that is still loading/unloading but the cargo transfer is completed
		cargoInfo.CompleteCargoTransfer();
		spot.OnCompleteCargoTransfer();		

		dcclite::Log::Trace("[Industry::OnCompleteSpotTransfer] {}: Spot {} finished transfer of {}", this->GetName(), spot.GetName(), cargoInfo.GetCargo().GetName());

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}	

	void Industry::OnCargoHolderStateChanged(AccessToken<detail::CargoHolder>) const
	{
		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this);
	}

	size_t Industry::FindCargoInfoIndexByCargoName(const std::string_view cargoName) const
	{
		RName cargoRName = RName::TryGetName(cargoName);
		if(!cargoRName)
			throw std::runtime_error(fmt::format("[Industry::FindCargoInfoIndexByCargoName] Cargo {} name not registered", cargoName));

		auto it = std::ranges::find_if(
			m_vecProduces, 
			[cargoRName](const detail::CargoInfo &ci) { return ci.GetCargo().GetName() == cargoRName; }
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
		auto cargoIndex = this->RandomSelectCargoToProduce();
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

		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this);
	}
}

