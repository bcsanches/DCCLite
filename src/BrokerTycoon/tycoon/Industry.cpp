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

	void Industry::OnCargoHolderStateChanged(AccessToken<detail::CargoHolder>) const
	{
		m_rclTycoon.OnObjectStateChanged(AccessToken<Industry>{}, *this);
	}
}

