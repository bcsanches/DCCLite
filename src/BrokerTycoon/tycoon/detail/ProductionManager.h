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

#include <dcclite/RName.h>

#include <rapidjson/document.h>

#include "CargoInfo.h"

namespace dcclite::broker::tycoon::detail
{
	class ProductionManager;
}

namespace dcclite::broker::tycoon
{
	class Cargo;
	class TycoonService;

	struct CargoQuantity
	{
		uint8_t m_uQuantity;
		uint8_t m_uReservedQuantity;
	};

	struct CargoIndex
	{		
		CargoIndex(const CargoIndex &) = default;
		CargoIndex &operator=(const CargoIndex &) = default;		

		private:
			friend class detail::ProductionManager;

			inline explicit CargoIndex(size_t index) :
				m_uIndex{ static_cast<uint32_t>(index) }
			{
				//should never happen, but just in case...
				assert(index < std::numeric_limits<uint32_t>::max());
			}

			[[nodiscard]] inline size_t GetIndex() const noexcept
			{
				return m_uIndex;
			}

			uint32_t m_uIndex;
	};
}

namespace dcclite::broker::tycoon::detail
{	
	/// <summary>
	/// A ProductionManager is responsible for managing the production of cargos in an industry. 
	/// 
	/// It keeps track of the cargos that can be produced, their production chances, and their current inventory. 
	/// 
	/// It also provides methods to load and save the production state, as well as to serialize the production data for communication with clients.
	/// </summary>
	class ProductionManager
	{
		public:
			void Load(TycoonService &tycoon, const rapidjson::Value &params, const RName industryName);

			const Cargo *TryGetCargoByCargoInfoIndex(CargoIndex index) const noexcept;

			std::optional<CargoIndex> TryGetCargoInfoIndexByCargoName(std::string_view name) const noexcept;
			std::optional<CargoIndex> TryGetCargoInfoIndexByCargoName(RName rname) const noexcept;

			[[nodiscard]] CargoIndex FindCargoInfoIndexByCargoName(RName cargoName, RName industryName) const;

			[[nodiscard]] unsigned CalculateTotalCargoStored() const noexcept;

			CargoIndex RandomSelectCargoToProduce(RName industryName) const noexcept;

			[[nodiscard]] CargoQuantity GetCargoQuantity(RName cargoName, RName industryName) const;

			[[nodiscard]] inline detail::CargoInfo &GetCargoInfo(CargoIndex index) noexcept
			{
				assert(index.GetIndex() < m_vecProduces.size());

				return m_vecProduces[index.GetIndex()];
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;
			void SerializeCargoInfoDelta(dcclite::JsonOutputStream_t &stream, CargoIndex cargoInfoIndex) const;

			void SaveState(dcclite::JsonOutputStream_t &stream) const;
			bool LoadState(const rapidjson::Value &params);

			void ResetState();

			[[nodiscard]] inline size_t GetProducesCount() const noexcept
			{
				return m_vecProduces.size();
			}

		private:
			void LoadProduce(TycoonService &tycoon, const rapidjson::Value &params);

			void AdjustProductionChances();

		private:
			std::vector<detail::CargoInfo>					m_vecProduces;
			unsigned										m_uTotalChance;
	};
}
