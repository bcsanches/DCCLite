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

#include <dcclite/IFolderObject.h>

#include <rapidjson/document.h>

#include "FastClock.h"
#include "FastClockThinker.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class Industry;
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

	enum class SpotStates 
	{
		FREE,
		RESERVED,
		LOADING,
		UNLOADING,
		CAR_PARKED
	};

	/**
	Car Spot State Machine:

			UNLOADING   ----
				^           \
				|            v
	FREE -> RESERVED    CAR_PARKED  ---
	   ^        |            ^         \
	   |		v            /         |
	   |	LOADING     -----          |
	   \	                           |
	    \                             /
	     -----------------------------
	
	
	*/

	class Spot: public INamedItem
	{
		public:
			Spot(RName name) :
				INamedItem{ name }
			{
				//empty
			}

			void Reserve(const char *info)
			{
				if (m_kState != SpotStates::FREE)
				{
					throw std::runtime_error("[Spot::Reserve] Spot is not free to reserve");
				}

				m_kState = SpotStates::RESERVED;

				if(info)
					m_strInformation = info;
				else
					m_strInformation.clear();
			}

			void Load()
			{
				if(m_kState != SpotStates::RESERVED)
				{
					throw std::runtime_error("[Spot::Load] Spot is not reserved to load");
				}

				m_kState = SpotStates::LOADING;
			}

			void Unload()
			{
				if (m_kState != SpotStates::RESERVED)
				{
					throw std::runtime_error("[Spot::Unload] Spot is not reserved to unload");
				}

				m_kState = SpotStates::UNLOADING;
			}

			void ParkCar()
			{
				if (m_kState != SpotStates::LOADING && m_kState != SpotStates::UNLOADING)
				{
					throw std::runtime_error("[Spot::ParkCar] Spot is not loading or unloading to park car");
				}

				m_kState = SpotStates::CAR_PARKED;
			}

			void RemoveCar()
			{
				if (m_kState != SpotStates::CAR_PARKED)
				{
					throw std::runtime_error("[Spot::RemoveCar] Spot does not have a car parked to remove");
				}

				m_kState = SpotStates::FREE;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;

		private:
			SpotStates m_kState = SpotStates::FREE;

			std::string m_strInformation;
	};

	class Industry : public Object
	{
		public:
			static const char *TYPE_NAME;

			Industry(RName name, TycoonService &tycoon, const rapidjson::Value &params);

			void ReserveSpot(const std::string_view spotName, const char *info);

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			~Industry() override = default;

			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;
			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:
			Spot *TryFindSpot(const std::string_view spotName);

		private:
			CargoHolder			m_clCargoHolder;
			std::vector<Spot>	m_vecSpots;

			TycoonService		&m_rclTycoon;
	};
}
