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

	class CargoHolder
	{		
		public:
			CargoHolder(TycoonService &tycoon, const rapidjson::Value &params);			

			void Consume(const FastClock &fastClock);		

		private:
			CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity, FastClock &fastClock) :
				m_clProductionThinker{ fastClock.MakeThinker("CargoHolder::ProductionThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker)) },
				m_rclFastClock{ fastClock },
				m_rclCargo{ cargo },
				m_fDailyRate{ dailyRate },
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

			FastClock					&m_rclFastClock;

			const Cargo &m_rclCargo;
			float		m_fDailyRate;
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

		private:
			CargoHolder m_clCargoHolder;
	};
}
