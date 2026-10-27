// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Pin.h"

namespace dcclite
{
	class Packet;
}

namespace Storage
{
	class EpromStream;
}

class OutputDecoderState
{
	private:
		uint16_t		m_uFlagsStorageIndex = 0;
		Pin				m_clPin;
		uint8_t			m_fFlags = 0;

	public:
		explicit OutputDecoderState(dcclite::Packet &packet);
		explicit OutputDecoderState(Storage::EpromStream &stream);	

	private:
		void Init();

};
