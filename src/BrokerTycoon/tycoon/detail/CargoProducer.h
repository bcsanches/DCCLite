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

#include <rapidjson/document.h>

#include "../FastClock.h"

#include "CargoInfo.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class FastClock;
	class Industry;
	class TycoonService;

	struct CargoQuantity
	{
		uint8_t m_uQuantity;
		uint8_t m_uReservedQuantity;
	};
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

			[[nodiscard]] unsigned CalculateTotalCargoStored() const noexcept;

			const Cargo *TryGetCargoByCargoInfoIndex(size_t index) const noexcept;
			int TryGetCargoInfoIndexByCargoName(std::string_view name) const noexcept;
			int TryGetCargoInfoIndexByCargoName(RName rname) const noexcept;

			[[nodiscard]] size_t FindCargoInfoIndexByCargoName(RName cargoName) const;

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

			void SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, const int cargoInfoIndex) const;

			void SerializeProductionDelta(dcclite::JsonOutputStream_t &stream) const;

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

			void LoadProductionData(TycoonService &tycoon, const rapidjson::Value &params);
			void LoadProduce(TycoonService &tycoon, const rapidjson::Value &params);

			void AdjustProductionChances();

			size_t RandomSelectCargoToProduce() const noexcept;

			void ScheduleProduction(const FastClock::time_point now);

			[[nodiscard]] inline detail::CargoInfo &GetCargoInfo(size_t index) noexcept
			{
				assert(index < m_vecProduces.size());

				return m_vecProduces[index];
			}

		private:
			std::vector<detail::CargoInfo>					m_vecProduces;

			FastClockThinker								m_clProductionThinker;

			Industry										&m_rclIndustry;

			unsigned										m_uProduceTotalChance;

			float											m_fpDailyRate;

			uint8_t											m_uMaxQuantity;

			bool											m_fProducing = false;
		};

}
