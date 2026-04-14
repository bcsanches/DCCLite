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

			/// <summary>
			/// Starts the cargo transfer, increments the reserved quantity and returns how long the transfer will take
			/// </summary>
			/// <returns>How long the transfer should takes</returns>
			std::chrono::hours StartCargoTransfer();

			void CargoTransferFinished();

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;

		private:
			CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity, std::chrono::hours transferTime, TycoonService &tycoon, FastClock &fastClock, const Industry &industry) :
				m_clProductionThinker{ fastClock.MakeThinker("CargoHolder::ProductionThinker", FAST_CLOCK_THINKER_LAMBDA(ProduceThinker)) },
				m_rclTycoon{ tycoon },
				m_rclFastClock{ fastClock },
				m_rclIndustry{ industry },
				m_rclCargo{ cargo },
				m_fpDailyRate{ dailyRate },
				m_uMaxQuantity{ maxQuantity },
				m_tTransferTime{ transferTime },
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

				if(m_tTransferTime <= std::chrono::hours{0})
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] transferTimeHours must be greater than zero");
				}
			}

			void ProduceThinker(FastClockDef::TimePoint_t tp);

			void ScheduleProduction();

			bool CanProduce() const noexcept
			{
				return m_uCurrentQuantity + m_uReservedQuantity < m_uMaxQuantity;
			}
			
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
			uint8_t		m_uReservedQuantity = 0;

			std::chrono::hours m_tTransferTime;

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
	   ^      | |            ^         \
	   |-----/	|            /         |
	   |        v           /          |
	   |	LOADING   ------           |
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

			void CancelReservation()
			{
				if (m_kState != SpotStates::RESERVED)
				{
					throw std::runtime_error("[Spot::CancelReservation] Spot is not reserved to cancel reservation");
				}

				m_kState = SpotStates::FREE;
				m_strInformation.clear();
			}

			inline bool CanLoad() const noexcept
			{
				return m_kState == SpotStates::RESERVED;
			}

			void Load();

			void Unload()
			{
				if (m_kState != SpotStates::RESERVED)
				{
					throw std::runtime_error("[Spot::Unload] Spot is not reserved to unload");
				}

				m_kState = SpotStates::UNLOADING;
			}

			void OnCargoTransferFinished()
			{
				if (m_kState != SpotStates::LOADING && m_kState != SpotStates::UNLOADING)
				{
					throw std::runtime_error("[Spot::OnCargoTransferFinished] Spot is not loading or unloading, cannot park car");
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
				m_strInformation.clear();
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;		

		private:			
			std::string			m_strInformation;			

			SpotStates m_kState = SpotStates::FREE;			
	};

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

		private:
			std::optional<size_t> TryFindSpotIndex(const std::string_view spotName) const;
			size_t FindSpotIndex(const std::string_view spotName) const;

			Spot *TryFindSpot(const std::string_view spotName);
			Spot &FindSpot(const std::string_view spotName);

			void SendSpotStateChangedEvent(const Spot &spot) const;
			void SendDeltaWithSpotStateChangedEvent(const Spot &spot) const;

			void OnSpotTransferFinished(FastClockDef::TimePoint_t tp, size_t spotIndex);

			void AddSpot(RName spotName);

		private:
			CargoHolder										m_clCargoHolder;
			std::vector<Spot>								m_vecSpots;

			//FIXME: this is ugly and sucks to dynamic allocate...
			std::vector<std::unique_ptr<FastClockThinker>>	m_vecSpotThinkers;

			TycoonService		&m_rclTycoon;
	};
}
