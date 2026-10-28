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

#include "Decoder.h"
#include "OutputDecoderState.h"


class OutputDecoder : public Decoder
{	
	private:
		OutputDecoderState m_clState;

	public:
		explicit OutputDecoder(dcclite::Packet &packet);
		explicit OutputDecoder(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream) override;

		bool IsOutputDecoder() const override
		{
			return true;
		}

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_OUTPUT;
		};

		bool AcceptServerState(dcclite::DecoderStates state, const unsigned long time) override;

		bool IsActive() const override
		{
			return m_clState.IsActive();
		}

		bool IsSyncRequired() const override
		{
			return false;
		}			
};
