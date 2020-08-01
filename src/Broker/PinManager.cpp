// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "PinManager.h"

#include "Decoder.h"

#include <stdexcept>

#include <fmt/format.h>

extern ArduinoBoards DecodeBoardName(std::string_view boardName)
{
	if(boardName.compare("ArduinoMega") == 0)
		return ArduinoBoards::MEGA;
	else if(boardName.compare("ArduinoUno") == 0)
		return ArduinoBoards::UNO;
	else
		throw std::logic_error(fmt::format("[DecodeBoardName] Invalid board {}", boardName));
}

static void ArduinoMega_FillPins(std::vector<PinManager::PinInfo> &vecPins)
{
	vecPins.resize(70);

	vecPins[0].m_pszSpecialName = "RX0";
	vecPins[1].m_pszSpecialName = "TX0";

	for(int i = 2; i <= 13; ++i)
		vecPins[i].m_pszSpecialName = "PWM";

	vecPins[14].m_pszSpecialName = "TX3";
	vecPins[15].m_pszSpecialName = "RX3";

	vecPins[16].m_pszSpecialName = "TX2";
	vecPins[17].m_pszSpecialName = "RX2";

	vecPins[18].m_pszSpecialName = "TX1";
	vecPins[19].m_pszSpecialName = "RX1";

	vecPins[20].m_pszSpecialName = "SDA";
	vecPins[21].m_pszSpecialName = "SCL";

	for (int i = 44; i <= 46; ++i)
		vecPins[i].m_pszSpecialName = "PWM";

	vecPins[50].m_pszSpecialName = "MISO";
	vecPins[51].m_pszSpecialName = "MOSI";
	vecPins[52].m_pszSpecialName = "SCK";
	vecPins[53].m_pszSpecialName = "SS";

	vecPins[54].m_pszSpecialName = "A01";
	vecPins[55].m_pszSpecialName = "A02";
	vecPins[56].m_pszSpecialName = "A03";
	vecPins[57].m_pszSpecialName = "A04";
	vecPins[58].m_pszSpecialName = "A05";
	vecPins[59].m_pszSpecialName = "A06";
	vecPins[60].m_pszSpecialName = "A07";
	vecPins[61].m_pszSpecialName = "A08";
	vecPins[62].m_pszSpecialName = "A09";
	vecPins[63].m_pszSpecialName = "A10";
	vecPins[64].m_pszSpecialName = "A11";
	vecPins[65].m_pszSpecialName = "A12";
	vecPins[66].m_pszSpecialName = "A13";
	vecPins[67].m_pszSpecialName = "A14";
	vecPins[68].m_pszSpecialName = "A15";
	vecPins[69].m_pszSpecialName = "A16";
}


static void ArduinoUno_FillPins(std::vector<PinManager::PinInfo> &vecPins)
{
	vecPins.resize(20);

	vecPins[0].m_pszSpecialName = "RX0";
	vecPins[1].m_pszSpecialName = "TX0";

	vecPins[3].m_pszSpecialName = "PWM";
	vecPins[5].m_pszSpecialName = "PWM";
	vecPins[6].m_pszSpecialName = "PWM";
	vecPins[9].m_pszSpecialName = "PWM";
	vecPins[10].m_pszSpecialName = "SS";
	vecPins[11].m_pszSpecialName = "PWM - MOSI";
	vecPins[12].m_pszSpecialName = "MISO";
	vecPins[13].m_pszSpecialName = "SCK";

	vecPins[14].m_pszSpecialName = "A01";
	vecPins[15].m_pszSpecialName = "A02";
	vecPins[16].m_pszSpecialName = "A03";
	vecPins[17].m_pszSpecialName = "A04";
	vecPins[18].m_pszSpecialName = "A05";	
}

PinManager::PinManager(ArduinoBoards board)
{
	switch (board)
	{
		case ArduinoBoards::MEGA:
			ArduinoMega_FillPins(m_vecPins);
			break;

		case ArduinoBoards::UNO:
			ArduinoUno_FillPins(m_vecPins);
			break;

		default:
			throw std::logic_error(fmt::format("[PinManager::PinManager] Unknown board: {}", board));
	}
}

void PinManager::RegisterPin(const Decoder &decoder, dcclite::BasicPin pin, const char *usage)
{
	if (!pin)
		throw std::invalid_argument(fmt::format("[PinManager::RegisterPin] Decoder tried to register an null pin to use as {}", decoder.GetName(), usage));

	if(pin.Raw() >= m_vecPins.size())
		throw std::out_of_range(fmt::format("[PinManager::RegisterPin] Decoder {} requested pin {} for {} that is out of range", decoder.GetName(), pin.Raw(), usage));

	auto &info = m_vecPins[pin.Raw()];
	if (info.m_pclUser != nullptr)
	{		
		throw std::invalid_argument(fmt::format(
			"[PinManager::RegisterPin] Decoder {} requested pin {} for {}, but is being used by {} for {}",
			decoder.GetName(),
			pin.Raw(),
			usage,
			info.m_pclUser->GetName(),
			info.m_pszUsage
		));
	}

	//all fine, register it
	info.m_pclUser = &decoder;
	info.m_pszUsage = usage;
}

void PinManager::UnregisterPin(const Decoder &decoder, dcclite::BasicPin pin)
{
	//ignore, like deleting a null pointer
	if(!pin)
		return;

	if (pin.Raw() >= m_vecPins.size())
		throw std::out_of_range(fmt::format("[PinManager::RegisterPin] Decoder {} released pin {} that is out of range", decoder.GetName(), pin.Raw()));

	auto &info = m_vecPins[pin.Raw()];
	if (info.m_pclUser == nullptr)
	{
		throw std::invalid_argument(fmt::format(
			"[PinManager::UnregisterPin] Decoder {} released pin {}, but it is not registered",
			decoder.GetName(),
			pin.Raw()			
		));
	}

	if (info.m_pclUser != &decoder)
	{
		throw std::invalid_argument(fmt::format(
			"[PinManager::UnregisterPin] Decoder {} released pin {}, but it used by {} for {}",
			decoder.GetName(),
			pin.Raw(),
			info.m_pclUser->GetName(),
			info.m_pszUsage
		));
	}

	info.m_pclUser = nullptr;
	info.m_pszUsage = nullptr;
}

void PinManager::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	auto pinsArray = stream.AddArray("pins");

	for (auto pinInfo : m_vecPins)
	{
		auto pinObj = pinsArray.AddObject();

		if(pinInfo.m_pclUser)
		{
			pinObj.AddStringValue("decoder", pinInfo.m_pclUser->GetName());
			pinObj.AddStringValue("usage", pinInfo.m_pszUsage);
		}

		if(pinInfo.m_pszSpecialName)
			pinObj.AddStringValue("specialName", pinInfo.m_pszSpecialName);
	}
}
