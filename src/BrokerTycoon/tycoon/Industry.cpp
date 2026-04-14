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

#include <magic_enum/magic_enum.hpp>

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
			std::chrono::hours{json::GetInt(params, "transferTimeHours", "[CargoHolder]")},
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

		if(!this->CanProduce())
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

	void Spot::Load()
	{
		if (!this->CanLoad())
		{
			throw std::runtime_error("[Spot::Load] Spot is not reserved to load");
		}

		m_kState = SpotStates::LOADING;		
	}

	///////////////////////////////////////////////////////////////////////////
	//
	//
	// Industry
	//
	//
	///////////////////////////////////////////////////////////////////////////

	const char *Industry::TYPE_NAME = "dcclite::broker::tycoon::Industry";

	void Industry::AddSpot(RName name)
	{
		m_vecSpots.emplace_back(name);

		m_vecSpotThinkers.push_back(m_rclTycoon.GetFastClock().MakeUniqueThinker(
			this->GetNameData(), 
			[this, spotIndex = m_vecSpots.size() - 1](FastClockDef::TimePoint_t tp)
			{
				this->OnSpotTransferFinished(tp, spotIndex);
			})
		);
	}

	Industry::Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params):
		Object{name},		
		m_clCargoHolder{tycoon, *this, params["produce"]},
		m_rclTycoon{ tycoon }
	{				
		if(auto spot = json::TryGetString(params, "spot"))
		{
			this->AddSpot(RName{ *spot });
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

		const auto spotsNum = spotsValue->value.GetArray().Size();
		if(spotsNum == 0)
		{
			throw std::invalid_argument("[Industry::Industry] at least one spot must be specified");
		}

		m_vecSpots.reserve(spotsNum);
		m_vecSpotThinkers.reserve(spotsNum);

		for(auto &it : spotsValue->value.GetArray())
		{
			if(!it.IsString())
			{
				throw std::invalid_argument("[Industry::Industry] each spot must be a string");
			}

			this->AddSpot(RName{ it.GetString() });			
		}
	}

	void Industry::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Object::Serialize(stream);

		m_clCargoHolder.Serialize(stream);
		
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

		m_clCargoHolder.SerializeDelta(stream);
	}

	std::optional<size_t> Industry::TryFindSpotIndex(const std::string_view spotName) const
	{
		auto it = std::ranges::find_if(m_vecSpots, [spotName](const Spot &s) { return s.GetNameData() == spotName; });
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

	Spot *Industry::TryFindSpot(const std::string_view spotName)
	{
		auto index = this->TryFindSpotIndex(spotName);
		
		return index ? &m_vecSpots[*index] : nullptr;		
	}

	Spot &Industry::FindSpot(const std::string_view spotName)
	{
		auto spot = this->TryFindSpot(spotName);
		if(spot == nullptr)
			throw std::runtime_error(fmt::format("[Industry::FindSpot] Spot {} not found in industry {}", spotName, this->GetName()));

		return *spot;
	}

	void Industry::SendSpotStateChangedEvent(const Spot &spot) const
	{
		m_rclTycoon.OnObjectStateChanged(IndustryToken{}, *this, [this, &spot](JsonOutputStream_t &stream)
			{
				//just send down the spot that changed...
				this->SerializeIdentification(stream);

				auto spotsData = stream.AddArray("spots");
				auto spotObject = spotsData.AddObject();
				spot.Serialize(spotObject);
			}
		);
	}

	void Industry::SendDeltaWithSpotStateChangedEvent(const Spot &spot) const
	{
		m_rclTycoon.OnObjectStateChanged(IndustryToken{}, *this, [this, &spot](JsonOutputStream_t &stream)
			{
				//just send down the delta, including cargo holder, because its state changed...
				this->SerializeDelta(stream);

				auto spotsData = stream.AddArray("spots");
				auto spotObject = spotsData.AddObject();
				spot.Serialize(spotObject);
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

	void Industry::StartSpotLoad(const std::string_view spotName)
	{
		const auto spotIndex = this->FindSpotIndex(spotName);
		auto &spot = m_vecSpots[spotIndex];

		//make sure spot will not throw after we call StartCargoTransfer on cargo holder, 
		// otherwise we will have an inconsistent state where cargo is reserved but spot is not loading
		if(!spot.CanLoad())
			throw std::runtime_error(fmt::format("[Industry::StartSpotLoad] Spot {} cannot be loaded because it is not reserved", spotName));

		auto transferTime = m_clCargoHolder.StartCargoTransfer();		
		spot.Load();

		dcclite::Log::Trace("[Industry::StartSpotLoad] {}: Started loading spot {}, transfer will take {}", this->GetName(), spotName, transferTime);

		m_vecSpotThinkers[spotIndex]->Schedule(m_rclTycoon.GetFastClock().Now() + transferTime);

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}

	void Industry::RemoveCarFromSpot(const std::string_view spotName)
	{
		auto &spot = this->FindSpot(spotName);

		spot.RemoveCar();

		this->SendSpotStateChangedEvent(spot);
	}

	void Industry::OnSpotTransferFinished(FastClockDef::TimePoint_t tp, size_t spotIndex)
	{
		assert(spotIndex < m_vecSpots.size());		

		auto &spot = m_vecSpots[spotIndex];		

		m_clCargoHolder.CargoTransferFinished();
		spot.OnCargoTransferFinished();

		dcclite::Log::Trace("[Industry::OnSpotFinishedTransfer] {}: Spot {} finished transfer", this->GetName(), spot.GetName());

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}	
}

