// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <chrono>

#include "../FastClock.h"

#include "ProductionManager.h"

namespace dcclite::broker::tycoon
{
	class Industry;
}

namespace dcclite::broker::tycoon::detail
{
	class Spot;

	class CargoProcessor
	{

	};	

	class CargoProducer : public CargoProcessor
	{
		public:
			CargoProducer(TycoonService &tycoon, Industry &industry, const rapidjson::Value &params);

			CargoProducer(const CargoProducer &) = delete;
			CargoProducer(CargoProducer &&) = delete;

			[[nodiscard]] inline bool IsProducing() const noexcept
			{
				return m_fProducing;
			}

			[[nodiscard]] inline unsigned CalculateTotalCargoStored() const noexcept
			{
				return m_clProductionManager.CalculateTotalCargoStored();
			}

			[[nodiscard]] inline const Cargo *TryGetCargoByCargoInfoIndex(CargoIndex index) const noexcept
			{
				return m_clProductionManager.TryGetCargoByCargoInfoIndex(index);
			}

			[[nodiscard]] inline std::optional<CargoIndex> TryGetCargoInfoIndexByCargoName(std::string_view name) const noexcept
			{
				return m_clProductionManager.TryGetCargoInfoIndexByCargoName(name);
			}

			[[nodiscard]] inline std::optional<CargoIndex> TryGetCargoInfoIndexByCargoName(RName rname) const noexcept
			{
				return m_clProductionManager.TryGetCargoInfoIndexByCargoName(rname);
			}			

			/**
			*	Start a transfer operation on spot with the cargo named by cargoName
			*
			*	Returns how long transfer will take.
			*/
			[[nodiscard]] std::chrono::hours StartSpotLoad(Spot &spot, RName cargoName);

			void FinishSpotTransfer(Spot &spot, const FastClock::time_point now);

			[[nodiscard]] CargoQuantity GetCargoQuantity(RName cargoName) const;

			///////////////////////////////////////////////////////////////////
			//
			// Serialization
			//
			///////////////////////////////////////////////////////////////////
			void Serialize(dcclite::JsonOutputStream_t &stream, const FastClock &fastClock) const;
			void SerializeDeltaDataOnly(dcclite::JsonOutputStream_t &stream, const FastClock &fastClock) const;

			void SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, CargoIndex cargoInfoIndex) const;

			inline void SerializeProductionDelta(dcclite::JsonOutputStream_t &stream) const
			{
				m_clProductionManager.SerializeDelta(stream);
			}

			///////////////////////////////////////////////////////////////////
			//
			// Save / Load
			//
			///////////////////////////////////////////////////////////////////
			void SaveState(dcclite::JsonOutputStream_t &stream) const;
			bool LoadState(const rapidjson::Value &params, const FastClock::time_point now);

			void ResetState(const FastClock::time_point now);

		private:
			void ProduceThinker(FastClockDef::TimePoint_t tp);			

			void ScheduleProduction(const FastClock::time_point now);			

		private:			
			FastClockThinker								m_clProductionThinker;

			ProductionManager								m_clProductionManager;

			Industry										&m_rclIndustry;			

			float											m_fpDailyRate;

			uint8_t											m_uMaxQuantity;

			bool											m_fProducing = false;
		};

}
