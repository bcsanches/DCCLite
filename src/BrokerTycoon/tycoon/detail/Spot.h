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

#include <dcclite/Object.h>

#include <rapidjson/document.h>

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
	                    +-----------+
			            | UNLOADING | ------
                        +-----------+       \
				              ^              \
						      |               \
							  | UNLOAD         |  Transfer completed
							  |                v
	+------+   RESERVE	+-----------+    +------------+   
	| FREE | ---------->|  RESERVED |    | CAR_PARKED | ---------------
	+------+            +-----------+    +------------+                 \
	   ^     CANCEL       |  |                 ^                         \
	   |<--------------- /   | LOAD            |                         |
	   |                     v                /                          | Car Removed
	   |                +---------+          / Transfer completed        |
	   |	            | LOADING |----------                            |
	   \	            +---------+                                      |
		\                                                               /
		 --------------------------------------------------------------


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

			[[nodiscard]] inline bool CanLoad() const noexcept
			{
				return m_kState == SpotStates::RESERVED;
			}

			[[nodiscard]] inline bool CanCompleteCargoTransfer() const noexcept
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

			void OnCompleteCargoTransfer(std::string cargoInformation)
			{
				if (!this->CanCompleteCargoTransfer())
				{
					throw std::runtime_error("[Spot::OnCompleteCargoTransfer] Spot is not loading or unloading, cannot park car");
				}

				m_strCargoInformation = std::move(cargoInformation);
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
				m_strCargoInformation.clear();
				m_iCargoIndex = -1;
			}

			[[nodiscard]] inline int GetCargoIndex() const noexcept
			{
				return m_iCargoIndex;
			}

			[[nodiscard]] inline bool IsTransfering() const noexcept
			{
				return m_kState == SpotStates::LOADING || m_kState == SpotStates::UNLOADING;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;

			void SaveState(dcclite::JsonOutputStream_t &stream, const Industry &industry) const;
			[[nodiscard]] bool LoadState(const rapidjson::Value &params, const Industry &industry);

			static std::optional<SpotStates> LoadStateEnum(const rapidjson::Value &params, const char *field);

			/// <summary>
			/// Expectd to be used only on load state when state is detected to be corrupted
			/// </summary>
			void Reset();

		private:
			std::string			m_strInformation;
			std::string			m_strCargoInformation;
			int					m_iCargoIndex = -1;

			SpotStates m_kState = SpotStates::FREE;
	};
}
