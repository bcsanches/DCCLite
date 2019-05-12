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

#include "Packet.h"

class Decoder;
class EpromStream;

namespace dcclite
{
	class Packet;
}

namespace DecoderManager
{
	Decoder *Create(const uint8_t slot, dcclite::Packet &packet);

	void Destroy(const uint8_t slot);
	void DestroyAll();

	void SaveConfig(EpromStream &stream);
	void LoadConfig(EpromStream &stream);

	/**
	Updates all decoders with the bitpack.

	Returns true if a state for any output decoder was processed.	
	*/
	bool ReceiveServerStates(const dcclite::StatesBitPack_t &changedStates, const dcclite::StatesBitPack_t &states);

	bool ProduceStatesDelta(dcclite::StatesBitPack_t &changedStates, dcclite::StatesBitPack_t &states);
	void WriteStates(dcclite::StatesBitPack_t &changedStates, dcclite::StatesBitPack_t &states);

	bool Update(const unsigned long ticks);
}
