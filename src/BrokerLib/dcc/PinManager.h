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

#include <string_view>
#include <vector>

#include <dcclite_shared/BasicPin.h>

#include <dcclite/Object.h>

namespace dcclite::broker
{

	class RemoteDecoder;

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
				const RemoteDecoder *m_pclUser;
				const char *m_pszUsage;

				const char *m_pszSpecialName;
			};

		private:	
			std::vector<PinInfo>	m_vecPins;
			ArduinoBoards			m_kBoard;

		public:
			explicit PinManager(ArduinoBoards board);

			void RegisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage);
			void UnregisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin);

			void Serialize(dcclite::JsonOutputStream_t &stream) const;
	};

}