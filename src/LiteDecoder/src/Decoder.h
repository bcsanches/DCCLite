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

#include "EmbeddedLibDefs.h"

typedef unsigned char Pin_t;

constexpr Pin_t null_pin = 255;

class EpromStream;

namespace dcclite
{
	class Packet;
}

class Decoder
{	
	public:
		Decoder(dcclite::Packet &packet);
		Decoder(EpromStream &stream) {}

		Decoder(const Decoder &) = delete;
		Decoder(const Decoder &&) = delete;
		Decoder &operator=(const Decoder &) = delete;

		virtual dcclite::DecoderTypes GetType() const = 0;

		virtual ~Decoder() = default;

		virtual void SaveConfig(EpromStream &stream)
		{
			//empty
		}		

		virtual bool AcceptServerState(dcclite::DecoderStates state) = 0;
};
