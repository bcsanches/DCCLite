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

#include "SharedLibDefs.h"

class EpromStream;

namespace dcclite
{
	class Packet;
}

class Decoder
{	
	public:
		explicit Decoder(dcclite::Packet &packet);
		explicit Decoder(EpromStream &stream) {}

		Decoder(const Decoder &) = delete;
		Decoder(const Decoder &&) = delete;
		Decoder &operator=(const Decoder &) = delete;

		virtual dcclite::DecoderTypes GetType() const = 0;
		virtual bool IsOutputDecoder() const = 0;

		//Calls update on every decoder.
		//Returns true if any state changed
		virtual bool Update(const unsigned long ticks)
		{
			return false;
		}

		//Returns true if decoder is activated (OUTPUT is On or Sensor has detected something)
		virtual bool IsActive() const = 0;

		/**
			Returns true if known remote state is not matching the current state

			Output decoders will always return false, as they do not need to sync

			Input decoders may return true or false, depends on the sync
		*/
		virtual bool IsSyncRequired() const = 0;

		virtual ~Decoder() = default;

		virtual void SaveConfig(EpromStream &stream)
		{
			//empty
		}		

		virtual bool AcceptServerState(dcclite::DecoderStates state) = 0;
};
