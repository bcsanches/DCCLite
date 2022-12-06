// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TurnoutDecoder.h"

#include "IDccLiteService.h"
#include "IDevice.h"

#include <Packet.h>

namespace dcclite::broker
{
	static std::uint8_t ReadFlag(const rapidjson::Value &params, const char *propertyName, std::uint8_t bit)
	{
		auto property = params.FindMember(propertyName);

		if (property == params.MemberEnd())
			return 0;

		return property->value.GetBool() ? bit : 0;
	}

	ServoTurnoutDecoder::ServoTurnoutDecoder(
		const DccAddress &address,
		const std::string &name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		TurnoutDecoder(address, name, owner, dev, params),
		m_clPin(params["pin"].GetInt())
	{
		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		assert(networkDevice);

		networkDevice->Decoder_RegisterPin(*this, m_clPin, "pin");

		auto powerPin = params.FindMember("powerPin");
		if (powerPin != params.MemberEnd())
		{
			m_clPowerPin = dcclite::BasicPin{ static_cast<dcclite::PinType_t>(powerPin->value.GetInt()) };
			networkDevice->Decoder_RegisterPin(*this, m_clPowerPin, "powerPin");
		}

		auto frogPin = params.FindMember("frogPin");
		if (frogPin != params.MemberEnd())
		{
			m_clFrogPin = dcclite::BasicPin{ static_cast<dcclite::PinType_t>(frogPin->value.GetInt()) };
			networkDevice->Decoder_RegisterPin(*this, m_clFrogPin, "frogPin");
		}

		m_fFlags |= ReadFlag(params, "inverted",			ServoTurnoutDecoderFlags::SRVT_INVERTED_OPERATION);
		m_fFlags |= ReadFlag(params, "ignoreSavedState",	ServoTurnoutDecoderFlags::SRVT_IGNORE_SAVED_STATE);
		m_fFlags |= ReadFlag(params, "activateOnPowerUp",	ServoTurnoutDecoderFlags::SRVT_ACTIVATE_ON_POWER_UP);

		m_fFlags |= ReadFlag(params, "invertedFrog",		ServoTurnoutDecoderFlags::SRVT_INVERTED_FROG);
		m_fFlags |= ReadFlag(params, "invertedPower",		ServoTurnoutDecoderFlags::SRVT_INVERTED_POWER);						

		auto range = params.FindMember("range");
		if (range != params.MemberEnd())
		{
			m_uEndPos = range->value.GetUint();
		}
		else
		{
			auto startPos = params.FindMember("startPos");

			if (startPos != params.MemberEnd())
			{
				m_uStartPos = startPos->value.GetUint();
				m_uEndPos = params["endPos"].GetUint();				

				if (m_uStartPos > m_uEndPos)
					std::swap(m_uStartPos, m_uEndPos);				
			}			
		}		

		this->CheckServoData(m_uStartPos, m_uEndPos, this->GetName());

		auto operationTime = params.FindMember("operationTime");
		m_tOperationTime = operationTime != params.MemberEnd() ? std::chrono::milliseconds{ operationTime->value.GetUint() } : m_tOperationTime;

		this->SyncRemoteState(
			(m_fFlags & ServoTurnoutDecoderFlags::SRVT_IGNORE_SAVED_STATE) && (m_fFlags & ServoTurnoutDecoderFlags::SRVT_ACTIVATE_ON_POWER_UP) ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE
		);
	}

	ServoTurnoutDecoder::~ServoTurnoutDecoder()
	{
		auto networkDevice = m_rclDevice.TryGetINetworkDevice();

		assert(networkDevice);

		networkDevice->Decoder_UnregisterPin(*this, m_clPin);
		networkDevice->Decoder_UnregisterPin(*this, m_clPowerPin);
		networkDevice->Decoder_UnregisterPin(*this, m_clFrogPin);
	}

	void ServoTurnoutDecoder::WriteConfig(dcclite::Packet &packet) const
	{
		TurnoutDecoder::WriteConfig(packet);

		packet.Write8(m_clPin.Raw());

		packet.Write8(m_fFlags);

		packet.Write8(m_clPowerPin.Raw());
		packet.Write8(m_clFrogPin.Raw());
		packet.Write8(m_uStartPos);
		packet.Write8(m_uEndPos);
		
		packet.Write8(TimeToTicks(m_tOperationTime, m_uStartPos, m_uEndPos));
	}

	void ServoTurnoutDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		TurnoutDecoder::Serialize(stream);

		stream.AddIntValue("pin", m_clPin.Raw());

		if (m_clPowerPin)
			stream.AddIntValue("powerPin", m_clPowerPin.Raw());

		if (m_clFrogPin)
			stream.AddIntValue("frogPin", m_clFrogPin.Raw());

		stream.AddIntValue("flags", m_fFlags);

		stream.AddIntValue("startPos", m_uStartPos);
		stream.AddIntValue("endPos", m_uEndPos);
		stream.AddIntValue("msOperationTime", static_cast<int>(m_tOperationTime.count()));
	}

	void ServoTurnoutDecoder::CheckServoData(const std::uint8_t startPos, const std::uint8_t endPos, std::string_view name)
	{
		if (startPos > endPos)
			throw std::logic_error(fmt::format("[ServoTurnoutDecoder::{}] [ValidateServoData] startPos must be < than endPos", name));

		if (startPos == endPos)
			throw std::logic_error(fmt::format("[ServoTurnoutDecoder::{}] [ValidateServoData] startPos must be < than endPos", name));
	}

	void ServoTurnoutDecoder::UpdateData(const std::uint8_t flags, const std::uint8_t startPos, const std::uint8_t endPos, const std::chrono::milliseconds operationTime)
	{
		CheckServoData(startPos, endPos, this->GetName());

		m_fFlags = flags;
		m_uStartPos = startPos;
		m_uEndPos = endPos;
		m_tOperationTime = operationTime;

		this->m_rclManager.Decoder_OnStateChanged(*this);
	}
}
