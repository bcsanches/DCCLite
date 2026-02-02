// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <dcclite/IFolderObject.h>

#include <rapidjson/document.h>

#include "FastClock.h"
#include "FastClockThinker.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class TycoonService;

	class IndustryToken
	{
		private:
			friend class Industry;
			friend class CargoHolder;

			IndustryToken() = default;
	};

	class CargoHolder
	{		
		public:
			CargoHolder(TycoonService &tycoon, const Industry &industry, const rapidjson::Value &params);

			void Consume(const FastClock &fastClock);

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;

		private:
			CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity, TycoonService &tycoon, FastClock &fastClock, const Industry &industry) :
				m_clProductionThinker{ fastClock.MakeThinker("CargoHolder::ProductionThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker)) },
				m_rclTycoon{ tycoon },
				m_rclFastClock{ fastClock },
				m_rclIndustry{ industry },
				m_rclCargo{ cargo },
				m_fpDailyRate{ dailyRate },
				m_uMaxQuantity{ maxQuantity },
				m_fProducing{ true }
			{
				if (dailyRate <= 0.0f)
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] dailyRate must be greater than zero");
				}

				if (maxQuantity == 0)
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] maxQuantity must be greater than zero");
				}
			}

			void ProduceThinker(FastClockDef::TimePoint_t tp);

			void ScheduleProduction();
			
		private:
			std::vector<std::string>	m_vecDestinations;

			FastClockThinker			m_clProductionThinker;

			TycoonService				&m_rclTycoon;
			FastClock					&m_rclFastClock;
			const Industry				&m_rclIndustry;

			const Cargo &m_rclCargo;
			float		m_fpDailyRate;
			uint8_t		m_uMaxQuantity;

			uint8_t		m_uCurrentQuantity = 0;

			bool		m_fProducing = false;
	};

	class Spot: public INamedItem
	{
		public:
			Spot(RName name) :
				INamedItem{ name }
			{
				//empty
			}

		private:

	};

	class Industry : public Object
	{
		public:
			static const char *TYPE_NAME;

			Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params);

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			~Industry() override = default;

			void Serialize(dcclite::JsonOutputStream_t &stream) const;

		private:
			CargoHolder m_clCargoHolder;
	};
}
