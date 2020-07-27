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

#include "BasicPin.h"

#include <string_view>
#include <vector>

class Decoder;

enum class ArduinoBoards
{
	MEGA,
	UNO
};

extern ArduinoBoards DecodeBoardName(std::string_view boardName);

/**
	This is really a helper class for helping managing boards and pin usage, not necessary for DCCLite system to work

*/
class PinManager
{
	public:
		struct PinInfo
		{
			inline PinInfo():
				m_pclUser{nullptr},
				m_pszUsage{nullptr},
				m_pszSpecialName{""}
			{
				//empty
			}
			const Decoder *m_pclUser;
			const char *m_pszUsage;

			const char *m_pszSpecialName;
		};

	private:	
		std::vector<PinInfo> m_vecPins;

	public:
		PinManager(ArduinoBoards board);

		void RegisterPin(const Decoder &decoder, dcclite::BasicPin pin, const char *usage);
		void UnregisterPin(const Decoder &decoder, dcclite::BasicPin pin);
};
