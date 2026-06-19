// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Spot.h"

#include <magic_enum/magic_enum.hpp>

#include <dcclite/JsonUtils.h>

#include "../Cargo.h"
#include "../Industry.h"

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
}
