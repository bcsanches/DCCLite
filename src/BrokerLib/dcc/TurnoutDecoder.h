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

#include "BasicPin.h"
#include "OutputDecoder.h"

namespace dcclite::broker
{

	class TurnoutDecoder : public OutputDecoder
	{
		public:
			TurnoutDecoder(
				const DccAddress& address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value& params
			):
				OutputDecoder(address, name, owner, dev, params)
			{
				//empty
			}

			bool IsTurnoutDecoder() const override
			{
				return true;
			}

			const char *GetTypeName() const noexcept override
			{
				return "TurnoutDecoder";
			}
	};

	class ServoTurnoutDecoder : public TurnoutDecoder
	{
		public:
			ServoTurnoutDecoder(
				const DccAddress& address,
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value& params
			);

			~ServoTurnoutDecoder() override;

			void WriteConfig(dcclite::Packet& packet) const override;

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_SERVO_TURNOUT;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			const char *GetTypeName() const noexcept override
			{
				return "ServoTurnoutDecoder";
			}

			[[nodiscard]] inline const std::uint8_t GetFlags() const noexcept
			{
				return m_fFlags;
			}

			[[nodiscard]] inline const std::uint8_t GetStartPosition() const noexcept
			{
				return m_uStartPos;
			}

			static inline uint8_t TimeToTicks(const std::chrono::milliseconds msec, const std::uint8_t startPos, const std::uint8_t endPos) noexcept
			{
				auto ticks = msec.count() / (endPos - startPos);

				return ticks > 255 ? 255 : static_cast<uint8_t>(ticks);				
			}

			void UpdateData(const std::uint8_t flags, const std::uint8_t startPos, const std::uint8_t endPos, const std::chrono::milliseconds operationTime);

			static void CheckServoData(const std::uint8_t startPos, const std::uint8_t endPos, std::string_view source);

		private:
			dcclite::BasicPin	m_clPin;
			dcclite::BasicPin	m_clPowerPin;
			dcclite::BasicPin	m_clFrogPin;

			std::uint8_t				m_uStartPos = 0;
			std::uint8_t				m_uEndPos = dcclite::SERVO_DEFAULT_RANGE;			
			std::chrono::milliseconds	m_tOperationTime = std::chrono::milliseconds{1000};

			std::uint8_t				m_fFlags = 0;			
	};
}
