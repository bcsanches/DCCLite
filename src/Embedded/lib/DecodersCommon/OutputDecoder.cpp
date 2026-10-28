// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "OutputDecoder.h"

#include <Arduino.h>

#include <dcclite_shared/SharedLibDefs.h>
#include <dcclite_shared/Packet.h>

#include "Console.h"
#include "Storage.h"

OutputDecoder::OutputDecoder(dcclite::Packet &packet) :
	Decoder::Decoder{ packet },
	m_clState{packet}	
{		
	//empty
}

OutputDecoder::OutputDecoder(Storage::EpromStream &stream):
	Decoder::Decoder{ stream },
	m_clState{ stream }
{	
	//empty
}


void OutputDecoder::SaveConfig(Storage::EpromStream &stream)
{
	Decoder::SaveConfig(stream);

	m_clState.SaveConfig(stream);
}

bool OutputDecoder::AcceptServerState(dcclite::DecoderStates state, const unsigned long time)
{
	return m_clState.AcceptServerState(state);	
}
