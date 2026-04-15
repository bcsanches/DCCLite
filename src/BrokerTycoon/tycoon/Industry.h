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
			void StartSpotLoad(const std::string_view spotName);
			void RemoveCarFromSpot(const std::string_view spotName);

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			~Industry() override = default;

			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;
			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			void OnCargoHolderStateChanged(AccessToken<detail::CargoHolder>) const;

		private:
			std::optional<size_t> TryFindSpotIndex(const std::string_view spotName) const;
			size_t FindSpotIndex(const std::string_view spotName) const;

			detail::Spot *TryFindSpot(const std::string_view spotName);
			detail::Spot &FindSpot(const std::string_view spotName);

			void SendSpotStateChangedEvent(const detail::Spot &spot) const;
			void SendDeltaWithSpotStateChangedEvent(const detail::Spot &spot) const;

			void OnSpotTransferFinished(FastClockDef::TimePoint_t tp, size_t spotIndex);

			void AddSpot(RName spotName);

		private:
			detail::CargoHolder								m_clCargoHolder;
			std::vector<detail::Spot>						m_vecSpots;

			//FIXME: this is ugly and sucks to dynamic allocate...
			std::vector<std::unique_ptr<FastClockThinker>>	m_vecSpotThinkers;

			TycoonService		&m_rclTycoon;
	};
}
