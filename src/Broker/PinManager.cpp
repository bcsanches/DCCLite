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

PinManager::PinManager(ArduinoBoards board)
{

}

void PinManager::RegisterPin(const Decoder &decoder, dcclite::BasicPin pin, const char *usage)
{
	if(pin >= m_vecPins.size())
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
	if (pin >= m_vecPins.size())
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