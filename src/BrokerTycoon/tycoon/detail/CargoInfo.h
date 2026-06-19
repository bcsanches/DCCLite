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

namespace dcclite::broker::tycoon
{	
	class Cargo;
	class TycoonService;
}

namespace dcclite::broker::tycoon::detail
{
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

			inline uint8_t GetQuantity() const noexcept
			{
				return m_uCurrentQuantity;
			}

			inline uint8_t GetReservedQuantity() const noexcept
			{
				return m_uReservedQuantity;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
			void SerializeDelta(dcclite::JsonOutputStream_t &stream) const;

			/// <summary>
			/// Starts the cargo transfer, increments the reserved quantity and returns how long the transfer will take
			/// </summary>
			/// <returns>How long the transfer should takes</returns>
			std::chrono::hours StartCargoTransfer();

			/// <summary>
			/// 
			/// </summary>
			/// <returns>A random destination for the cargo</returns>
			[[nodiscard]] const std::string &CompleteCargoTransfer();

			void SaveState(dcclite::JsonOutputStream_t &stream) const;
			void LoadState(const rapidjson::Value &params);

			/// <summary>
			/// Expectd to be used only on load state when state is detected to be corrupted
			/// </summary>
			void Reset();

		private:
			void LoadDestinations(const rapidjson::Value &params);

		private:
			std::vector<std::string>	m_vecDestinations;

			const Cargo &m_rclCargo;
			std::chrono::hours			m_tTransferTime;
			unsigned					m_uSequence = 0;
			uint8_t						m_u8Chance;

			uint8_t						m_uCurrentQuantity = 0;
			uint8_t						m_uReservedQuantity = 0;
	};	
}
