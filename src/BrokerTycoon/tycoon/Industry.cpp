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
		m_rclTycoon{ tycoon },
		m_clProducer{tycoon, *this, params}
	{		
		this->LoadSpots(params);
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
		const auto spotIndex = this->FindSpotIndex(spotName);	
		auto &spot = m_vecSpots[spotIndex];

		auto transferTime = m_clProducer.StartSpotLoad(spot, cargoName);

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

		m_clProducer.FinishSpotTransfer(spot, m_rclTycoon.GetFastClock().Now());		

		this->SendDeltaWithSpotStateChangedEvent(spot);
	}

	void Industry::OnCargoProduced(AccessToken<detail::CargoProducer> token, unsigned cargoIndex)
	{
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

		m_clProducer.Serialize(stream, m_rclTycoon.GetFastClock());

		auto spotsData = stream.AddArray("spots");
		for (auto &spot : m_vecSpots)
		{
			auto spotObject = spotsData.AddObject();
			spot.Serialize(spotObject);
		}
	}

	void Industry::SerializeDelta(dcclite::JsonOutputStream_t &stream, int cargoInfoHintIndex) const
	{
		this->SerializeIdentification(stream);

		m_clProducer.SerializeDeltaDataOnly(stream, m_rclTycoon.GetFastClock());		

		if (cargoInfoHintIndex < 0)
		{
			m_clProducer.SerializeProductionDelta(stream);			
		}
		else
		{
			m_clProducer.SerializeCargoInfo(stream, cargoInfoHintIndex);
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
					m_clProducer.SerializeCargoInfo(stream, cargoInfoIndex);
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
		m_clProducer.SaveState(stream);		

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
		if (!m_clProducer.LoadState(params, m_rclTycoon.GetFastClock().Now()))
			goto CORRUPTED_STATE;

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

		return;

CORRUPTED_STATE:	

		//try to fix state
		dcclite::Log::Warn("[Industry::LoadState] [{}] State corrupted, resetting it to initial state, sorry...", this->GetName());

		m_clProducer.ResetState(m_rclTycoon.GetFastClock().Now());		
			
		for (size_t i = 0, sz = m_vecSpots.size(); i < sz; ++i)
		{
			m_vecSpots[i].Reset();
			m_vecSpotThinkers[i]->Cancel();
		}		
	}
}

