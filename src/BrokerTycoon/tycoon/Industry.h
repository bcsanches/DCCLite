// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <vector>

#include <dcclite/AccessToken.h>
#include <dcclite/IFolderObject.h>

#include <rapidjson/document.h>

#include "FastClock.h"
#include "FastClockThinker.h"

#include "detail/Industry_detail.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class Industry;
	class TycoonService;

	class Industry : public Object
	{
		public:
			static const char *TYPE_NAME;

			Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params);

			void ReserveSpot(const std::string_view spotName, const char *info);
			void CancelSpotReservation(const std::string_view spotName);
			void StartSpotLoad(const std::string_view spotName, const std::string_view cargoName);
			void RemoveCarFromSpot(const std::string_view spotName);

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			~Industry() override = default;

			void SerializeDelta(dcclite::JsonOutputStream_t &stream, int cargoInfoHintIndex = -1) const;
			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			void SaveState(dcclite::JsonOutputStream_t &stream) const;
			void LoadState(const rapidjson::Value &params);

		private:
			void SerializeDeltaDataOnly(dcclite::JsonOutputStream_t &stream) const;
			void SerializeCargoInfo(dcclite::JsonOutputStream_t &stream, const int cargoInfoIndex) const;

			std::optional<size_t> TryFindSpotIndex(const std::string_view spotName) const;
			size_t FindSpotIndex(const std::string_view spotName) const;

			detail::Spot *TryFindSpot(const std::string_view spotName);
			detail::Spot &FindSpot(const std::string_view spotName);

			void SendSpotStateChangedEvent(const detail::Spot &spot) const;
			void SendDeltaWithSpotStateChangedEvent(const detail::Spot &spot) const;

			void OnCompleteSpotTransfer(FastClockDef::TimePoint_t tp, size_t spotIndex);

			void AddSpot(RName spotName);

			void ProduceThinker(FastClockDef::TimePoint_t tp);
			void ScheduleProduction();

			void LoadProductionData(const rapidjson::Value &params);
			void LoadProduce(const rapidjson::Value &params);
			void LoadSpots(const rapidjson::Value &params);

			void AdjustProductionChances();

			size_t RandomSelectCargoToProduce() const noexcept;

			size_t FindCargoInfoIndexByCargoName(const std::string_view cargoName) const;

			unsigned CalculateTotalCargoStored() const noexcept;

		private:			
			std::vector<detail::Spot>						m_vecSpots;
			std::vector<detail::CargoInfo>					m_vecProduces;

			//FIXME: this is ugly and sucks to dynamic allocate...
			std::vector<std::unique_ptr<FastClockThinker>>	m_vecSpotThinkers;

			FastClockThinker								m_clProductionThinker;

			TycoonService									&m_rclTycoon;

			unsigned										m_uProduceTotalChance;

			float											m_fpDailyRate;

			uint8_t											m_uMaxQuantity;
			bool											m_fProducing = false;
	};
}
