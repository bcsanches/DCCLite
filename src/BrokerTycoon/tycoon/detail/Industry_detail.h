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

#include <dcclite/Object.h>
#include <dcclite/RName.h>

#include "../FastClockDefs.h"
#include "../FastClockThinker.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class FastClock;
	class Industry;
	class TycoonService;
}

namespace dcclite::broker::tycoon::detail
{
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

	class Spot : public INamedItem
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

				if (info)
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

			inline bool CanCompleteCargoTransfer() const noexcept
			{
				return (m_kState == SpotStates::LOADING || m_kState == SpotStates::UNLOADING);
			}


			void Load(int cargoIndex);

			void Unload()
			{
				if (m_kState != SpotStates::RESERVED)
				{
					throw std::runtime_error("[Spot::Unload] Spot is not reserved to unload");
				}

				m_kState = SpotStates::UNLOADING;
			}

			void OnCompleteCargoTransfer()
			{
				if (this->CanCompleteCargoTransfer())
				{
					throw std::runtime_error("[Spot::OnCompleteCargoTransfer] Spot is not loading or unloading, cannot park car");
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
				m_iCargoIndex = -1;
			}

			int GetCargoIndex() const noexcept
			{
				return m_iCargoIndex;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;

		private:
			std::string			m_strInformation;
			int					m_iCargoIndex = -1;

			SpotStates m_kState = SpotStates::FREE;
	};

	class CargoInfo
	{
		public:
			CargoInfo(const TycoonService &tycoon, const rapidjson::Value &params);

			inline uint8_t GetChance() const noexcept
			{
				return m_u8Chance;
			}

			inline const Cargo &GetCargo() const noexcept
			{
				return m_rclCargo;
			}

			inline unsigned GetSequence() const noexcept
			{
				return m_uSequence;
			}

			inline void SetSequence(unsigned v) noexcept
			{
				m_uSequence = v;
			}

			inline void IncreaseQuantity() noexcept
			{
				m_uCurrentQuantity++;
			}			

			inline unsigned GetTotal() const noexcept
			{
				return m_uCurrentQuantity + m_uReservedQuantity;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;

			/// <summary>
			/// Starts the cargo transfer, increments the reserved quantity and returns how long the transfer will take
			/// </summary>
			/// <returns>How long the transfer should takes</returns>
			std::chrono::hours StartCargoTransfer();

			void CompleteCargoTransfer();

		private:
			void LoadDestinations(const rapidjson::Value &params);

		private:
			std::vector<std::string>	m_vecDestinations;	

			const Cargo					&m_rclCargo;
			std::chrono::hours			m_tTransferTime;			
			unsigned					m_uSequence = 0;
			uint8_t						m_u8Chance;

			uint8_t						m_uCurrentQuantity = 0;
			uint8_t						m_uReservedQuantity = 0;
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
			CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity, std::chrono::hours transferTime, FastClock &fastClock, const Industry &industry);

			void ProduceThinker(FastClockDef::TimePoint_t tp);

			void ScheduleProduction();

			bool CanProduce() const noexcept
			{
				return m_uCurrentQuantity + m_uReservedQuantity < m_uMaxQuantity;
			}

		private:
			std::vector<std::string>	m_vecDestinations;

			FastClockThinker			m_clProductionThinker;

			FastClock &m_rclFastClock;
			const Industry &m_rclIndustry;

			const Cargo &m_rclCargo;
			float		m_fpDailyRate;
			uint8_t		m_uMaxQuantity;

			uint8_t		m_uCurrentQuantity = 0;
			uint8_t		m_uReservedQuantity = 0;

			std::chrono::hours m_tTransferTime;

			bool		m_fProducing = false;
	};
}
