// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CargoInfo.h"

#include <numeric>
#include <random>

#include <dcclite/JsonUtils.h>

#include "../Cargo.h"
#include "../TycoonService.h"

static thread_local std::mt19937 g_clRandomGenerator{ std::random_device{}() };

namespace dcclite::broker::tycoon::detail
{
	///////////////////////////////////////////////////////////////////////////
	//
	//
	// CargoInfo
	//
	//
	///////////////////////////////////////////////////////////////////////////
	CargoInfo::CargoInfo(const TycoonService &tycoon, const rapidjson::Value &params) :
		m_rclCargo{ tycoon.FindCargoByName(RName::Get(json::GetString(params, "cargo", "[Tycoon::detail::CargoInfo]"))) },
		m_tTransferTime{ std::chrono::hours{ json::GetInt(params, "transferTimeHours", "[CargoHolder]") } }
	{
		if (m_tTransferTime <= std::chrono::hours{ 0 })
		{
			throw std::invalid_argument("[CargoHolder::CargoInfo] transferTimeHours must be greater than zero");
		}

		auto chance = json::TryGetDefaultInt(params, "chance", 1);
		if (chance < 1 || chance > 255)
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
}
